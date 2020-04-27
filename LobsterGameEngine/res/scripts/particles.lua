require("./chicken")


function OnBegin()
    particleSystem = Lobster.GetParticleComponent(this)
	rigidbody = Lobster.GetPhysicsComponent(this)
	print("onBegin called")
end

function OnUpdate(dt)
    if (Lobster.IsMouseDown(1)) then
        particleSystem:EmitOnce()
        print("Button down detected, and fuck ocornut.")
        print(global_X)
    end
end

function OnClick()
end