#include "camera_controller.h"

CameraController::CameraController()
{
	BaseNode.AddChild(HeadNode);

	HeadNode.MoveD(41.0f/32.0f);

	HeadNode.AddChild(HandNode);
	HandNode.MoveD(-0.25f);
	HandNode.MoveH(-0.25f);

	auto d = BaseNode.GetDVector();
	auto f = BaseNode.GetHPostVector();
	auto v = BaseNode.GeVVector();

	auto hp = HandNode.GetWorldPosition();
	auto hv = Vector3Transform({ 0, 1, 0 }, HeadNode.GetWorldMatrix());

	auto delta = Vector3Subtract(hv, hp);

	int j = 0;
}

void CameraController::Update(const WADFile::LevelMap& map)
{
	float runspeed = 18.0f * GetFrameTime();

	//rotate the player base left and right
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		BaseNode.RotateD(GetMouseDelta().x * -0.125f);

	// rotate the camera up and down
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		HeadNode.RotateH(GetMouseDelta().y * 0.125f);

	// move the player based on it's local transform
	if (IsKeyDown(KEY_W))
		BaseNode.MoveV(runspeed);

	if (IsKeyDown(KEY_S))
		BaseNode.MoveV(-runspeed);

	if (IsKeyDown(KEY_A))
		BaseNode.MoveH(-runspeed);

	if (IsKeyDown(KEY_D))
		BaseNode.MoveH(runspeed);

	Vector3 pos = BaseNode.GetWorldPosition();

	size_t sector = map.GetSectorFromPoint(pos.x, pos.y);
	if (sector != size_t(-1))
		BaseNode.SetPosition(pos.x, pos.y, map.Sectors->Contents[sector].Floor);
}

Vector3 CameraController::GetPosition()
{
	return BaseNode.GetWorldPosition();
}

void CameraController::SetPosition(const Vector3& pos)
{
	BaseNode.SetPosition(pos);
}

void CameraController::SetCamera(Camera3D& camera)
{
	// where the camera node is in the world
	camera.position = HeadNode.GetWorldPosition();

	// where the depth vector of the camera node is in world space
	camera.target = Vector3Transform({ 0, 1, 0}, HeadNode.GetWorldMatrix());
}

