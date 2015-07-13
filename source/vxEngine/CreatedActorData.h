#pragma once

struct Entity;

namespace Component
{
	struct Actor;
	struct Physics;
}

struct CreatedActorData
{
	Entity* entity;
	Component::Actor* componentActor;
	Component::Physics* componentPhysics;
};