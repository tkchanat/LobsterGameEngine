
function OnBegin()
	mesh = Lobster.GetMeshComponent(this)
	mesh:SetTimeMultiplier(0.4)
	mesh:PlayAnimation()
end

function OnUpdate(dt)

end
