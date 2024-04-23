#include "ParticleSwarm.h"
#include "CoreMinimal.h"
#include "igl/default_num_threads.h"
#include "Log/Log.h"
#include "Math/Random.h"
#include "igl/parallel_for.h"
#include "spdlog/stopwatch.h"


void ParticleSwarm::Init()
{
    Position.resize(GroupSize, Dimention);
    Velocity.resize(GroupSize, Dimention);
	CurrentEnergy.resize(GroupSize, EnergyDimention);
    InitBound();

    VelMax = (PositionMax - PositionMin) * 0.1;
	spdlog::stopwatch sw;
	for (int i = 0;i < GroupSize;i ++)
	{
		for (int j = 0; j < Dimention; j++)
			Velocity(i, j) = 2 * VelMax(j) * (Random::Random() - 0.5);
	}
	InitPosition();
	LOG_INFO("Init done ,takes time: {}", sw);

    GroupBestEnergy = CurrentEnergy;
    GroupBest = Position;
	GlobalBest = GroupBest.row(0);
	GlobalBestEnergy = GroupBestEnergy.row(0);
    UpdateGlobalBest();
}

void ParticleSwarm::InitPosition()
{
	std::tie(Position, CurrentEnergy) = RandomValidParameters(GroupSize);
}

void ParticleSwarm::InitBound()
{
    PositionMin = Eigen::VectorXd::Constant(Dimention, 0.);
    PositionMax = Eigen::VectorXd::Constant(Dimention, 1.);
}

bool ParticleSwarm::Compare(const PEnergy& A, const PEnergy& B)
{
	for(int i = 0; i < A.size(); i ++)
		if(A[i] == B[i]) continue;
		else return A[i] < B[i];
	return false;
}

std::tuple<ParticleSwarm::GroupParameter, ParticleSwarm::GroupPEnergy> ParticleSwarm::RandomValidParameters(int Count)
{
	ParticleSwarm::GroupParameter Result;
	Result.resize(Count, Dimention);
	ParticleSwarm::GroupPEnergy Energy;
	Energy.resize(Count, EnergyDimention);
	std::atomic_int Index = 0;
	std::atomic_bool Done = false;
	spdlog::stopwatch t;
	const size_t nthreads = std::thread::hardware_concurrency();
	ParallelFor(nthreads, [&](int i) {
		ParticleSwarm::Parameter Temp;
		Temp.resize(Dimention);
		PEnergy TempEnergy;
		while(!Done)
		{
			for (int j = 0; j < Dimention; j++)
			{
				Temp(j) = (PositionMax(j) - PositionMin(j)) * Random::Random() + PositionMin(j);
			}
			bool Valid = false;
			TempEnergy = CalcEnergy(Temp, Valid, true);
			if(Valid) {
				int PreIndex = Index.fetch_add(1);
				if(PreIndex >= Count) return;
				if(PreIndex >= Count - 1) Done = true;
				Result.row(PreIndex) = Temp;
				Energy.row(PreIndex) = TempEnergy;
				LOG_INFO("Find no.{}, time: {}, {}",int(PreIndex), t, Temp);
				t.reset();
			}
		}
	})
	return {Result, Energy};
}

ParticleSwarm::Parameter ParticleSwarm::RandomOneParameter() const
{
	ParticleSwarm::Parameter Temp; Temp.resize(Dimention);
	for (int j = 0; j < Dimention; j++)
	{
		Temp(j) = (PositionMax(j) - PositionMin(j)) * Random::Random() + PositionMin(j);
	}
	return Temp;
}

void ParticleSwarm::UpdatePosition()
{
    Position = Position + Velocity;
	for (int i = 0; i < GroupSize; i++)
	{
		for (int j = 0; j < Dimention; j++)
		{
			if (Position(i, j) < PositionMin(j))
				Position(i, j) = (PositionMax(j) - PositionMin(j)) * Random::Random() * 0.1 + PositionMin(j);
			else if (Position(i, j) > PositionMax(j))
				Position(i, j) = (PositionMax(j) - PositionMin(j)) * Random::Random() * (-0.1) + PositionMax(j);
		}
	}
}

void ParticleSwarm::UpdateVelocity()
{
	for (int i = 0; i < GroupSize; i++)
	{
		Velocity.row(i) = w * Velocity.row(i) + c1 * Random::Random() * (GroupBest.row(i) - Position.row(i))
			+ c2 * Random::Random() * (GlobalBest - Position.row(i));
	}

	// min < V < max
	for (int i = 0; i < GroupSize; i++)
	{
		for (int j = 0; j < Dimention; j++)
		{
			if (Velocity(i, j) < -VelMax(j))
				Velocity(i, j) = -VelMax(j);
			if (Velocity(i, j) > VelMax(j))
				Velocity(i, j) = VelMax(j);
		}
	}
}

void ParticleSwarm::UpdateEngery()
{
    CurrentEnergy.resize(GroupSize, EnergyDimention);
	ParallelFor(GroupSize, [this](int i){
		bool Valid;
	    CurrentEnergy.row(i) = CalcEnergy(Position.row(i), Valid, false);
	}, 10);
}

void ParticleSwarm::UpdateGroupBest()
{
	int Updated = 0;
	for (int i = 0; i < GroupSize; i++)
	{
		if (Compare(CurrentEnergy.row(i) , GroupBestEnergy.row(i)))
		{
			Updated ++;
			GroupBest.row(i) = Position.row(i);
            GroupBestEnergy.row(i) = CurrentEnergy.row(i);
		}
	}
	LOG_INFO("Update group best particle count: {}", Updated);
}

void ParticleSwarm::UpdateGlobalBest()
{
	static int LastUpdatedRound = 0;
	static int CurrentRound = 0;

	CurrentRound ++;

	bool Updated = false;
	for (int i = 0; i < GroupSize; i++)
	{
		if (Compare(GroupBestEnergy.row(i), GlobalBestEnergy)){
			LastUpdatedRound = CurrentRound;
			Updated = true;
			GlobalBest = GroupBest.row(i);
        	GlobalBestEnergy = GroupBestEnergy.row(i);
		}
	}
    if(Updated) LOG_INFO("Update global best solution: energy {0} \n {1}", GlobalBestEnergy, GlobalBest);
	else LOG_INFO("Global best solution not updated: energy {0}, not update iteration {1}", GlobalBestEnergy, CurrentRound - LastUpdatedRound);
}

ParticleSwarm::Parameter ParticleSwarm::Optimize(bool &Result)
{
	LOG_INFO("Initializing... ");
	Init();

	LOG_INFO("Running ... ");
	for (int i = 0; i < Iteration; i++)
	{
		spdlog::stopwatch Watch;
		LOG_INFO("Running iteration {0} / {1} :", i, Iteration);
		UpdateVelocity();
		UpdatePosition();
		UpdateEngery();
		UpdateGroupBest();
		UpdateGlobalBest();
		LOG_INFO("Iteration {} takes time :{}" ,i, Watch);
	}
	Result = true;
	return GlobalBest;
}