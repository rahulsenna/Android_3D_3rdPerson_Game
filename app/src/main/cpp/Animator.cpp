
#include "Animator.h"
#include "AndroidOut.h"

Animator::Animator(Animation* animation)
{
	m_CurrentTime = 0.0;
	m_CurrentAnimation = animation;

	m_FinalBoneMatrices.reserve(animation->m_Bones.size());
	aout << "m_FinalBoneMatrices.size(): " << m_FinalBoneMatrices.size()  << std::endl;

	for (int i = 0; i < animation->m_Bones.size(); i++)
		m_FinalBoneMatrices.emplace_back(glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt)
{
	m_DeltaTime = dt;
	if (m_CurrentAnimation)
	{
		m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
		CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
	}
}

void Animator::PlayAnimation(Animation* pAnimation)
{
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
	glm::mat4 globalTransformation;
	Bone* Bone = m_CurrentAnimation->FindBone(node->name);
	if (Bone)
	{
		Bone->Update(m_CurrentTime);
		globalTransformation = parentTransform * Bone->m_LocalTransform;
	} else
	{
		globalTransformation = parentTransform * node->transformation;
	}

	auto &boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
	int index = boneInfoMap[node->name].id;
	if (index != -1)
	{
		m_FinalBoneMatrices[index] = globalTransformation * boneInfoMap[node->name].offset;	
	}

	for (int i = 0; i < node->childrenCount; i++)
		CalculateBoneTransform(&node->children[i], globalTransformation);
}
