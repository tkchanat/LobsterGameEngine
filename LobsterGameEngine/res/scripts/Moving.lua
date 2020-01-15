function OnBegin()
	-- do nothing
end

sign = 1

function OnUpdate(dt)
	-- do nothing
	dx = dt / 50
	transform.WorldPosition.x = transform.WorldPosition.x + dx * sign
	if (transform.WorldPosition.x > 5) then
		sign = -sign
		transform.WorldPosition.x = 5
	elseif (transform.WorldPosition.x < -5) then
		sign = -sign
		transform.WorldPosition.x = -5
	end

end
