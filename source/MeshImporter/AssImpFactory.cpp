#include <assimp/scene.h>
#include <vxEngineLib/AnimationFile.h>
#include <vxEngineLib/FileFactory.h>

void exportAnimation(const aiAnimation* anim)
{
	vx::Animation animation;
	animation.layerCount = anim->mNumChannels;
	animation.layers = std::make_unique<vx::AnimationLayer[]>(animation.layerCount);

	for (auto j = 0u; j < anim->mNumChannels; ++j)
	{
		auto channel = anim->mChannels[j];

		VX_ASSERT(channel->mNumPositionKeys == channel->mNumRotationKeys);
		VX_ASSERT(channel->mNumPositionKeys == channel->mNumScalingKeys);

		auto frameCount = channel->mNumPositionKeys;

		vx::AnimationLayer layer;
		layer.frameCount = frameCount;
		layer.frameRate = 30;
		layer.samples = std::make_unique<vx::AnimationSample[]>(frameCount);

		for (u32 i = 0; i < frameCount; ++i)
		{
			auto position = channel->mPositionKeys[i];
			auto rotation = channel->mRotationKeys[i];
			auto q = rotation.mValue;

			vx::AnimationSample sample;
			sample.frame = i;
			sample.transform.m_translation = vx::float3(position.mValue.x, position.mValue.y, position.mValue.z);
			sample.transform.m_rotation = vx::float4(q.x, q.y, q.z, q.w);
			sample.transform.m_scaling = 1.0f;

			//printf("position: %f %f %f ", position.mValue.x, position.mValue.y, position.mValue.z);
			//printf("rotation: %f %f %f %f\n", q.x,q.y,q.z, q.w);

			layer.samples[i] = sample;
		}

		animation.layers[j] = std::move(layer);
	}


	vx::AnimationFile animFile(std::move(animation), anim->mName.C_Str());

	std::string animFileName = anim->mName.C_Str();
	animFileName += ".animation";

	vx::FileFactory::saveToFile(animFileName.c_str(), &animFile);
	printf("Saved animation file '%s'\n", animFileName.c_str());

}