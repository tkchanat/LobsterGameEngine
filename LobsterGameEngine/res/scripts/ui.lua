function OnBegin()
    score = 0
    landed = true
    chicken = scene:GetGameObjectByName('chicken')
	particle = scene:GetGameObjectByName('particle')
	floor = scene:GetGameObjectByName('floor')
end

function OnUpdate(dt)
    if (chicken:Intersects(particle) and landed) then
        landed = false
        score = score + 1
    end
    
    if (chicken:Intersects(floor)) then
		--landing
		landed = true
	end
end

