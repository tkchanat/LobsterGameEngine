function OnBegin()
end

function OnUpdate(dt)
	ds = dt / 50
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

end
