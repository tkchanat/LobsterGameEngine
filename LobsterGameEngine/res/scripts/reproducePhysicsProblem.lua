function OnBegin()
    Lobster.RemoveGameObject(scene, this)
    start = 0
end

function OnUpdate(dt)
    ds = dt / 50
    if (start == 0) then 
	--if (Lobster.RayIntersect(scene:GetGameCamera(), Lobster.GetPhysicsComponent(this), 10000)) then
        rigidbody:ApplyForce(Lobster.Vec3(0, 0.5, 0))
    start = 1
    end
end
