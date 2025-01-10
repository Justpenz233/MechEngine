//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
#include "Render/Core/transform_data.h"

class TransformComponent;
namespace MechEngine::Rendering
{
	using namespace luisa;
	using namespace luisa::compute;

	class ENGINE_API TransformSceneProxy : public SceneProxy
	{
	public:
		explicit TransformSceneProxy(GpuScene& InScene);

		virtual void UploadDirtyData(luisa::compute::Stream& stream) override;

		/**
		 * Try to add a new transform to the scene, if the transform already exists, return the existing id
		 * @param InTransform Transformcomponent to add
		 * @return The transform id
		 */
		uint AddTransform(TransformComponent* InTransform);

		void BindTransform(uint InstanceID, uint TransformID);

		/**
		 * Check if a transform exists in the scene
		 * @param InTransform Transformcomponent to check
		 * @return True if the transform exists
		 */
		bool IsExist(TransformComponent* InTransform) const;

		/**
		 * Check if a transform exists in the scene
		 * @param TransformID The transform id to check
		 * @return True if the transform exists
		 */
		bool IsExist(uint TransformID) const;

		/**
		* Try to update a transform render data, the transform may not exist in the scene
		* @param InTransform Transformcomponent to update
		*/
		void UpdateTransform(TransformComponent* InTransform);

		[[nodiscard]] FORCEINLINE uint GetTransformCount() const noexcept;

		[[nodiscard]] UInt get_instance_transform_id(Expr<uint> instance_id) const
		{
			return bindelss_buffer<uint>(instance_to_transform_bid)->read(instance_id);
		}

		[[nodiscard]] Var<transform_data> get_transform_data(Expr<uint> transform_id) const
		{
			return bindelss_buffer<transform_data>(transform_data_bid)->read(transform_id);
		}

		[[nodiscard]] Var<transform_data> get_instance_transform_data(Expr<uint> instance_id) const
		{
			$comment("read instance transform data");
			return get_transform_data(get_instance_transform_id(instance_id));
		}

	protected:
		uint Id = 0;
		vector<uint> Instance2Transformid;
		vector<transform_data> TransformDatas;
		map<TransformComponent*, uint> TransformIdMap;

		set<TransformComponent*> DirtyTransforms;
		set<TransformComponent*> NewTransforms;

		map<uint, uint> TransformToInstanceId;// should be one to many

		uint transform_data_bid;
		uint instance_to_transform_bid;
		BufferView<transform_data> transform_buffer;
		BufferView<uint> instance_to_transform_buffer;
	};

	uint TransformSceneProxy::GetTransformCount() const noexcept
	{
		return Id;
	}
}