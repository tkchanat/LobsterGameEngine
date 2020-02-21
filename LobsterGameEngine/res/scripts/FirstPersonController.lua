function OnBegin()
	rigidbody = Lobster.GetPhysicsComponent(this)
	-- lock cursor
	--[[
	Lobster.LockCursor()
	Lobster.DisableCursor()
	
	cursorLocked = true
	eventTriggered = false
	jumped = false
	]]--
end

function OnUpdate(dt)
	ds = dt / 50
	-- toggle cursor lock (for debug only)
	--[[
	if (Lobster.IsKeyUp(string.byte("L"))) then eventTriggered = false; end
	if (not eventTriggered and Lobster.IsKeyDown(string.byte("L"))) then
		if (cursorLocked) then
			Lobster.UnlockCursor()
			Lobster.EnableCursor()
			cursorLocked = false
		else
			Lobster.LockCursor()
			Lobster.DisableCursor()
			cursorLocked = true
		end
		eventTriggered = true
	end	
	-- rotate according to cursor position
	if (cursorLocked) then
		dx = Lobster.GetMouseDeltaX() * ds
		dy = Lobster.GetMouseDeltaY() * ds
		transform:RotateEuler(-dy, transform:Right())
		transform:RotateEuler(-dx, Lobster.Vec3(0, 1, 0))		
	end
	--]]
	-- up
	if (Lobster.IsKeyDown(265) or Lobster.IsKeyDown(string.byte("W"))) then
		dir = transform:Forward(); dir.y = 0
		dir = Lobster.normalize(dir)
		transform.WorldPosition.x = transform.WorldPosition.x - dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y - dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z - dir.z * ds
	end
	-- down
	if (Lobster.IsKeyDown(264) or Lobster.IsKeyDown(string.byte("S"))) then
		dir = transform:Forward(); dir.y = 0
		dir = Lobster.normalize(dir)
		transform.WorldPosition.x = transform.WorldPosition.x + dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y + dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z + dir.z * ds
	end
	-- left
	if (Lobster.IsKeyDown(263) or Lobster.IsKeyDown(string.byte("A"))) then
		dir = transform:Right(); dir.y = 0
		dir = Lobster.normalize(dir)
		transform.WorldPosition.x = transform.WorldPosition.x - dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y - dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z - dir.z * ds
	end
	-- right
	if (Lobster.IsKeyDown(262) or Lobster.IsKeyDown(string.byte("D"))) then
		dir = transform:Right(); dir.y = 0
		dir = Lobster.normalize(dir)
		transform.WorldPosition.x = transform.WorldPosition.x + dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y + dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z + dir.z * ds
	end
	-- jump (space)
	if Lobster.IsKeyUp(32) then jumped = false end
	if (not jumped and Lobster.IsKeyDown(32)) then
		if (rigidbody ~= nil) then
			rigidbody:ApplyForce(Lobster.Vec3(0.0, 1, 0.0), Lobster.Vec3(0, 10, 0))
			jumped = true
		end
	end

end
