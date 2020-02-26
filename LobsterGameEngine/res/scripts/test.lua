intTest = 10
floatTest = tonumber(string.format("%.3f", 10.123))
stringTest = "HelloWorld"

function OnBegin()
	print("onBegin called")
	-- finalTest = "final"
end

function OnUpdate(dt)
	if Lobster.RayIntersect(scene:GetGameCamera(), Lobster.GetPhysicsComponent(this), 10000) then
		print("mouse on enter this :))")
	end
end

function OnClick()
	print("Button down detected, and fuck ocornut.")
end