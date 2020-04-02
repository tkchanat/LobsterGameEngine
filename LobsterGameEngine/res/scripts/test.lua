function OnBegin()
	rigidbody = Lobster.GetPhysicsComponent(this)
	y = 0
	started = false
end

function OnUpdate(dt)
	ds = dt / 50
	if (Lobster.RayIntersect(scene:GetGameCamera(), Lobster.GetPhysicsComponent(this), 10000)) then
		started = true
		y = y+1
	elseif (y~=0 and y~= nil) then
		print(y)
		rigidbody:AddVelocity(Lobster.Vec3(0, 0, -10))
		y=0
	end

	--landed

end
