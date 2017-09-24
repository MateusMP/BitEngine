#pragma once

#include "MyGameEntitySystem.h"

class Character
{
public:
	Character(u32 _id)
		: id(_id)
	{}

	virtual void Create(MyGameEntitySystem* esys)
	{
		entity = esys->createEntity();
		transform = esys->AddComponent<BitEngine::Transform3DComponent>(entity);
		renderable = esys->AddComponent<BitEngine::RenderableMeshComponent>(entity);
	}

	void setPosition(const glm::vec3& pos){
		transform->setPosition(pos);
	}

	u32 getID() const{
		return id;
	}

	BitEngine::EntityHandle getEntity() const {
		return entity;
	}

	BitEngine::ComponentRef<BitEngine::Transform3DComponent>& getTransform() {
		return transform;
	}

protected:
	BitEngine::EntityHandle entity;
	BitEngine::ComponentRef<BitEngine::Transform3DComponent> transform;
	BitEngine::ComponentRef<BitEngine::RenderableMeshComponent> renderable;

private:
	u32 id;
};

class OverworldCharacter : public Character
{
public:
	OverworldCharacter(u32 id) 
		: Character(id)
	{}

private:

};

class Player
{
public:
	Player(const std::string& nick, u32 netid)
		: m_nickname(nick), m_netID(netid), m_character(nullptr)
	{
	}

	const std::string& getNickname() const {
		return m_nickname;
	}

	u32 getNetID() const{
		return m_netID;
	}

	void setCharacter(Character* character){
		m_character = character;
	}

private:
	std::string m_nickname;
	u32 m_netID;

	Character* m_character;

};