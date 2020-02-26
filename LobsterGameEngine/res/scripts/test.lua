
function OnBegin()
	camera = Lobster.GetCameraComponent(this)
	ui = camera:GetUI()
	spriteList = ui:GetSpriteList()
end

function OnUpdate(dt)
	if(Lobster.IsMouseHold()) then
		Lobster.GetMouseDeltaY()
		rigidbody = Lobster.GetPhysicsComponent(this)
		
	end
end