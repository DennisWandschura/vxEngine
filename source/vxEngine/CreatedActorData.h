#pragma once

struct EntityActor;

namespace Component
{
	struct Actor;
	struct Physics;
}

struct CreatedActorData
{
	EntityActor* entity;
	Component::Actor* componentActor;
	Component::Physics* componentPhysics;
};