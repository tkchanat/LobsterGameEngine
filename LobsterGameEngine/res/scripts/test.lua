intTest = 10
floatTest = tonumber(string.format("%.3f", 10.123))
stringTest = "HelloWorld"

function OnBegin()
	--[[
	camera = Lobster.GetCameraComponent(this)
	ui = camera:GetUI()
	spriteList = ui:GetSpriteList()
	]]--
	print("onBegin called")
	audio = Lobster.GetAudioSource(this)
	audio:Play()
	-- finalTest = "final"
end

function OnUpdate(dt)

end

function OnClick()
	print("Button down detected, and fuck ocornut.")
end