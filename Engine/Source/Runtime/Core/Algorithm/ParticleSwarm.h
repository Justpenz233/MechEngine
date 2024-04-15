#pragma once
#include "Object/Object.h"
#include <Eigen/Eigen>

/**
 * @brief A particle swarm method, by default assume all value in [0, 1]
 */
class ParticleSwarm : public Object
{
public:
    using Parameter = Eigen::RowVectorXd;
    using GroupParameter = Eigen::MatrixXd;
    using PEnergy = Eigen::RowVectorXd;
    using GroupPEnergy = Eigen::MatrixXd;

    ParticleSwarm(int _Dimention, int _EnergyDimention, int _Iteration = 100, int _GroupSize = 100)
    :Dimention(_Dimention),EnergyDimention(_EnergyDimention), Iteration(_Iteration), GroupSize(_GroupSize) {};

    // Random a parameter, not guarantee valid
    Parameter RandomOneParameter() const;

    /**
     * @brief Optimize
     * 
     * @tparam T Result, should implement constructor T(const VectorXd&)
     * @param Result If find a valid solution
     * @return T Best solution
     */
    Parameter Optimize(bool& Result);

protected:
    int Dimention;
    int EnergyDimention;
    int GroupSize = 100;
    int Iteration = 100;

    double w = 0.9;
    double c1 = 2;
    double c2 = 2;

    bool FindValid = false;

    Parameter VelMax;
    Parameter PositionMax;
    Parameter PositionMin;

    Parameter GlobalBest;
    PEnergy GlobalBestEnergy;

    GroupParameter GroupBest;
    GroupPEnergy GroupBestEnergy;

    GroupParameter Position;
    GroupParameter Velocity;
    GroupPEnergy CurrentEnergy;

    void Init();

    virtual void InitPosition();

    virtual void InitBound();

    virtual bool Compare(const PEnergy& A, const PEnergy& B);
    
    // Override this
    virtual PEnergy CalcEnergy(const Parameter& Particle, bool& Valid, bool StopInValid = false) = 0;

    // Random a valid parameter with energy
	std::tuple<GroupParameter, GroupPEnergy> RandomValidParameters(int Count);
    
    void UpdateVelocity();

    void UpdatePosition();

    void UpdateEngery();

    void UpdateGroupBest();

    void UpdateGlobalBest();
};
