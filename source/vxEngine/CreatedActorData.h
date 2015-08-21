#pragma once

struct EntityActor;

namespace Component
{
	struct Actor;
}

struct CreatedActorData
{
	EntityActor* entity;
	Component::Actor* componentActor;
};