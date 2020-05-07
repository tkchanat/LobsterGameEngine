
function OnBegin()	
	--Lobster.SetBlend(true, Lobster.Vec3(0.8, 0.4, 0.1), 0.4)
	Lobster.SetBlend(false, Lobster.Vec3(0.8, 0.4, 0.1), 0.4)
	Lobster.ApplySobel(false, 0.3)
	Lobster.ApplyKernel(true, Lobster.Vec3(0, -1, 0), Lobster.Vec3(-1, 5, -1), Lobster(0, -1, 0))
	Lobster.SetSSR(false)
end

function OnUpdate(dt)

end