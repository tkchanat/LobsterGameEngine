-- This is a script attached to the camera, controlling the sprite and ingame "loading"
-- Never call "this" in this script!!! No GameObject set!!!

function OnBegin()	
	cameraObj = scene:GetGameObjectByName('Main Camera')
	bg = scene:GetGameObjectByName('court')
	camera = Lobster.GetCameraComponent(cameraObj)	
	ui = camera:GetUI()
	startBtn = ui:GetSpriteByLabel('startBtn')
	Lobster.SetBlur(true)
	Lobster.SetBlend(true, Lobster.Vec3(1, 1, 1), 0.2)
end

function OnClick()
    if startBtn == nil then
        Lobster.SetBlend(true, Lobster.Vec3(1, 0, 0), 0.7)
        return
    end
    ui:RemoveSprite(startBtn)
    bgm = Lobster.GetAudioSource(bg)
    bgm:Play()
    Lobster.SetBlur(false)
    Lobster.SetBlend(false, Lobster.Vec3(0, 0, 0), 0)
end
