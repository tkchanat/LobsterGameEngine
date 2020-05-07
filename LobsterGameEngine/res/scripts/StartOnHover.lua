-- Just to play the audio when the button is hovered

function OnBegin()
	cameraObj = scene:GetGameObjectByName('Main Camera')
	btnSound = Lobster.GetAudioSource(cameraObj)
end

function OnHover()
	if btnSound then
		btnSound:Play()
	end
end