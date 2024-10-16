#include "simple_logger.h"
#include "gfc_matrix.h"
#include "entity.h"

typedef struct
{
	Entity *entityList;
	Uint32	entityMax;
}EntityManager;

static EntityManager entity_manager = {0};

//put in think and update when you get the time

void entity_system_close()
{
	for (int i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse)continue; //skip ones not set
		entity_free(&entity_manager.entityList[i]); //clean them all up
	}
	free(entity_manager.entityList);
	memset(&entity_manager, 0, sizeof(EntityManager));
}

void entity_system_init(Uint32 maxEnts)
{
	if (entity_manager.entityList)
	{
		slog("entity manager already exists");
		return;
	}
	if (!maxEnts)
	{
		slog("cannot allocate 0 entities for the entity manager");
		return;
	}
	entity_manager.entityList = gfc_allocate_array(sizeof(Entity), maxEnts);
	if (!entity_manager.entityList)
	{
		slog("failed to allocate %i entities for the entity manager", maxEnts);
		return;
	}

	entity_manager.entityMax = maxEnts;
	atexit(entity_system_close);
}

void entity_draw(Entity *self)
{
	GFC_Matrix4 matrix;
	if (!self)return;

	if(self->draw) self->draw(self);

	gfc_matrix4_from_vectors(
		matrix,
		self->position,
		self->rotation,
		self->scale);

	gf3d_model_draw(
		self->model,
		matrix,
		GFC_COLOR_WHITE,
		0);
}

void entity_draw_all()
{
	for (int i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse)continue; //skip ones not set
		entity_draw(&entity_manager.entityList[i]);
	}
}

Entity *entity_new()
{
	for (int i = 0; i < entity_manager.entityMax;i++)
	{
		if (entity_manager.entityList[i]._inuse) continue; //skip ones in use
		memset(&entity_manager.entityList[i], 0, sizeof(Entity)); //clear it out in case
		entity_manager.entityList[i]._inuse = 1;
		entity_manager.entityList[i].scale = gfc_vector3d(1,1,1);
		//any default values should be set
		return &entity_manager.entityList[i];
	}
	return NULL; //no more entity slots
}

void entity_free(Entity *self)
{
	if (!self) return;

	//free up anything that might have been allocated FOR this
	if (self->free) self->free(self);
	gf3d_model_free(self->model);
	memset(self, 0, sizeof(Entity)); 
}