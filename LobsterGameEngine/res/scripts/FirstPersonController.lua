function OnBegin()
	-- do nothing
end

function OnUpdate(dt)
	ds = dt / 50
	-- up
	if (Lobster.IsKeyDown(265)) then
		dir = transform:Forward()
		transform.WorldPosition.x = transform.WorldPosition.x - dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y - dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z - dir.z * ds
	end
	-- down
	if (Lobster.IsKeyDown(264)) then
		dir = transform:Forward()
		transform.WorldPosition.x = transform.WorldPosition.x + dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y + dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z + dir.z * ds
	end
	-- left
	if (Lobster.IsKeyDown(263)) then
		dir = transform:Right()
		transform.WorldPosition.x = transform.WorldPosition.x - dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y - dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z - dir.z * ds
	end
	-- right
	if (Lobster.IsKeyDown(262)) then
		dir = transform:Right()
		transform.WorldPosition.x = transform.WorldPosition.x + dir.x * ds
		transform.WorldPosition.y = transform.WorldPosition.y + dir.y * ds
		transform.WorldPosition.z = transform.WorldPosition.z + dir.z * ds
	end

end
