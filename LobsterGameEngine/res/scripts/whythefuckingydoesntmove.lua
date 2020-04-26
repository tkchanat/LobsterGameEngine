function OnBegin()
	rigidbody = Lobster.GetPhysicsComponent(this)
end

function OnUpdate(dt)
	ds = dt / 50
	--if (Lobster.RayIntersect(scene:GetGameCamera(), Lobster.GetPhysicsComponent(this), 10000)) then
	if (Lobster.IsKeyUp(32)) then
		rigidbody:ApplyForce(Lobster.Vec3(0, 1, 0))
	end
end
