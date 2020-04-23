function OnBegin()
	var = 0x5235fc71
	ido = scene:GetGameObjectById(var) -- Never use this, bugs
	nameo = scene:GetGameObjectByName('Sphere') -- Use this, but ensure the name to be unique

	print(nameo)
	
	Lobster.SetBlur(true)
	-- The three vec3 are the COLUMNS forming the filter kernel
	Lobster.ApplyKernel(true, Lobster.Vec3(0, -1, 0), Lobster.Vec3(-1, 4, -1), Lobster.Vec3(0, -1, 0))
	-- To remove the kernel filter, click "Settings > Renderer > Clear Applied filters"
end

function OnUpdate(dt)

end
