#pragma once

#include "object_transform.h"
#include "doom_map.h"

class CameraController
{
protected:
	ObjectTransform BaseNode;
	ObjectTransform HeadNode;
	ObjectTransform HandNode;

public:
	CameraController();
	virtual ~CameraController() = default;

	void Update(const WADFile::LevelMap& map);

	Vector3 GetPosition();

	void SetPosition(const Vector3& pos);

	void SetCamera(Camera3D& camera);
};