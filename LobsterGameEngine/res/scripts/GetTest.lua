
function OnBegin()	
	sphere = scene:GetGameObjectByName('Sphere')
	print(sphere)
	script = Lobster.GetScript(sphere)
	display = 0	
end

function OnUpdate(dt)
	display = script:GetInt('var')
end
