function OnBegin()
	print("new game")
	
	score = 0
	chicken = scene:GetGameObjectByName('chicken')
	rigidbody = Lobster.GetPhysicsComponent(chicken)
	floor = scene:GetGameObjectByName('floor')
	hoop = scene:GetGameObjectByName('hoop')
	court = scene:GetGameObjectByName('court')
	audio = Lobster.GetAudioSource(chicken)
	particle = scene:GetGameObjectByName('particle')
	particleSystem = Lobster.GetParticleComponent(particle)
	-- mesh = Lobster.GetMeshComponent(this)
	-- mesh:SetTimeMultiplier(0.4)
	-- mesh:PlayAnimation()
	
	shot = false
	shooting_force_y = 0
	shooting_force_x = 0
	initial_x = transform.WorldPosition.x
	initial_y = transform.WorldPosition.y
	initial_z = transform.WorldPosition.z
	shooting_initial_y = 1
	landing_y = 0.5
	floor_count = 10
end


function resetGame()
	shot = false
	floor_count = 20
	shooting_force_y = 0
	shooting_force_x = 0  
	transform.WorldPosition.x = initial_x
	transform.WorldPosition.y = initial_y
	transform.WorldPosition.z = initial_z
	rigidbody:StopObject()
end

function OnUpdate(dt)
	ds = dt / 50
	--if (Lobster.RayIntersect(scene:GetGameCamera(), Lobster.GetPhysicsComponent(this), 10000)) then
	if (Lobster.IsMouseDown(0)) then
		dx = Lobster.GetMouseDeltaX() * ds
		dy = Lobster.GetMouseDeltaY() * ds
		shooting_force_x = shooting_force_x + dx
		shooting_force_y = shooting_force_y + dy
	elseif (shooting_force_y ~= 0 and shooting_force_y ~= nil and shooting_force_x ~= nil) then
		
		audio:Play()
		transform.WorldPosition.y = transform.WorldPosition.y + shooting_initial_y
		rigidbody:AddVelocity(Lobster.Vec3(-shooting_force_x/50, 1.65, 0.75))
		shooting_force_y = 0
		shooting_force_x = 0
		shot = true
	end

	if (chicken:Intersects(particle) and shot) then
		--goal
		particleSystem:EmitOnce()
		score = score + 1
		shot = false
	end

	if (chicken:Intersects(floor)) then
		--landing
		floor_count = floor_count - 1

		if floor_count == 0 then
			resetGame()
		end
	end

	if (chicken:Intersects(hoop) or chicken:Intersects(court)) then
		--stuck at the hoop
		floor_count = floor_count - 1
		if shot then
			audio:Play()
			shot = false
		end

		if floor_count == 0 then
			resetGame()
		end
	end
end

