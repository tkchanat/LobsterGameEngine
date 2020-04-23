function OnBegin()
    particleSystem = Lobster.GetParticleComponent(this)
	print("onBegin called")
end

function OnUpdate(dt)
    if (Lobster.IsMouseDown(1)) then
        particleSystem:EmitOnce()
        print("Button down detected, and fuck ocornut.")
    end
end

function OnClick()
end