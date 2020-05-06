-- This is a script attached to the camera, controlling the sprite and ingame "loading"
-- Never call "this" in this script!!! No GameObject set!!!

function OnBegin()	
	cameraObj = scene:GetGameObjectByName('Main Camera')
	camera = Lobster.GetCameraComponent(cameraObj)
	ui = camera:GetUI()
	startBtn = ui:GetSpriteByLabel('startBtn')
	Lobster.SetBlur(true)
end

function OnClick()
	ui:RemoveSprite(startBtn)	
	Lobster.SetBlur(false)
end
