//program.js

//todo:
//image/font/shader delete on highlight and delete
//clear images/fonts/shaders on new keyboard reload
//import videos (the program will show it, just haven't added it to the web version)
//add keys / delete keys (later)



var canvas = document.getElementById("myCanvas");
var ctx = canvas.getContext("2d");

canvas.width = 3840;
canvas.height = 1100;

canvas.style.width = Math.floor(canvas.width / 4) + "px";
canvas.style.height = Math.floor(canvas.height / 4) + "px";

class key {
	constructor(z, x, y, w, h, isRect, RGBA, outputType, outputValue, 
				affectedByCapsLock, useImage, text, font, shader, pressShader,
				modType, modText, modShader, modOutputValue){
		this.Z = z; this.X = x; this.Y = y; this.W = w; this.H = h;
		this.isRect = isRect; this.RGBA = RGBA; this.outputType = outputType; this.outputValue = outputValue;
		this.affectedByCapsLock = affectedByCapsLock; this.useImage = useImage;
		this.text = text; this.font = font; this.shader = shader; this.pressShader = pressShader;
		this.modType = modType; this.modText = modText; this.modShader = modShader; this.modOutputValue = modOutputValue;
		this.pluginEvent = -1;
	}
}

class obj {
	constructor(z, x, y, w, h, RGBA, freeType, text, font, shader){
		this.Z = z; this.X = x; this.Y = y; this.W = w; this.H = h;
		this.RGBA = RGBA; this.freeType = freeType; this.text = text; this.font = font; this.shader = shader;
	}
}

class keyboardLayout {
	constructor(layoutName){
		this.layoutName = layoutName;
		this.fonts = new Array();
		this.globalOffsetX = 0;
		this.globalOffsetY = 0;
		this.backgroundImgPath = "";
		this.backgroundIsVideo = false;
		this.backgroundColor = "#FF00FFFF";
		this.shaderGlobal = -1;
		this.pressShaderGlobal = -1;
		this.shaders = new Array();
		this.keys = new Array();
		this.freeObj = new Array();
		this.plugins = new Array();
	}
}

function newKeyboard(layoutName){
	keyboard = new keyboardLayout(layoutName);
	keyboard.keys = new Array;
	imageList = new Array();
	fontList = new Array();
	shaderList = new Array();
	shaderFileList = new Array();
	pluginList = new Array();
	//remove images, fonts, and shaders + fonts and shaders in select
	let lib = document.getElementById("imageLibrary");
	while (lib.children.length > 1){
		lib.removeChild(lib.children[1]);
	}
	lib = document.getElementById("fontLibrary");
	while (lib.children.length > 1){
		lib.removeChild(lib.children[1]);
	}
	lib = document.getElementById("shaderLibrary");
	while (lib.children.length > 1){
		lib.removeChild(lib.children[1]);
	}
	
	let select = document.getElementById("fontPicker");
	while (select.options.length > 1){
		select.remove(1);
	}
	select = document.getElementById("shaderPicker");
	while (select.options.length > 1){
		select.remove(1);
	}
	select = document.getElementById("pressShaderPicker");
	while (select.options.length > 1){
		select.remove(1);
	}
	select = document.getElementById("shaderPickerGlobal");
	while (select.options.length > 1){
		select.remove(1);
	}
	select = document.getElementById("pressShaderPickerGlobal");
	while (select.options.length > 1){
		select.remove(1);
	}
	for (let k = 0; k < modShaderElements.length; k++){
		while (modShaderElements[k].options.length > 1)
			modShaderElements[k].remove(1);
	}
	
	closeKey();
}

var keyboard;
var selectedKey = -1;
var imageList;
var fontList;
var shaderList;
var shaderFileList; 
var pluginList;
const numberOfModifiers = 5
var modTypeElements = new Array();
var modValueElements = new Array();
var modTextElements = new Array();
var modShaderElements = new Array();

var globalParamsOpen = false;

function start(){
	addModEvents();

	newKeyboard("myLayout");
	newKeyboardANSI(false);
//	addMacroPad();
//	addBigPad();
}

function renderKeys(){
	//clear canvas
	canvas.style.backgroundColor = keyboard.backgroundColor;
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	
	let gX = keyboard.globalOffsetX;
	let gY = keyboard.globalOffsetY;
	
	//render background image:
	if (keyboard.backgroundImgPath.length > 0){
			let imageToDraw = getImageFromName(keyboard.backgroundImgPath);
			if (imageToDraw != null)
				ctx.drawImage(imageToDraw, 0, 0, canvas.width, canvas.height);
	}
	
	//render all keyboard.keys
	for (let i = 0; i < keyboard.keys.length; i++){
		if (keyboard.keys[i].isRect){
			ctx.fillStyle = keyboard.keys[i].RGBA;
			ctx.fillRect(keyboard.keys[i].X + gX, keyboard.keys[i].Y + gY, keyboard.keys[i].W, keyboard.keys[i].H);
		}
		else if (keyboard.keys[i].useImage){
			let imageToDraw = getImageFromName(keyboard.keys[i].text);
			if (keyboard.keys[i].affectedByCapsLock && capsLock){
				imageToDraw = getImageFromName(keyboard.keys[i].modText[0]);
			}
			for (let j = 0; j < modDown.length; j++){
				if (modDown[j] && keyboard.keys[i].modText[j].length > 0){
					if (j == 0 && keyboard.keys[i].affectedByCapsLock && capsLock){
						imageToDraw = getImageFromName(keyboard.keys[i].text);	//get normal text
						continue;
					}
					imageToDraw = getImageFromName(keyboard.keys[i].modText[j]);
				}
			}
			if (imageToDraw != null)
				ctx.drawImage(imageToDraw, keyboard.keys[i].X + gX, keyboard.keys[i].Y + gY, keyboard.keys[i].W, keyboard.keys[i].H);
			else{
				ctx.strokeStyle = "red";
				ctx.lineWidth = 5;
				ctx.strokeRect(keyboard.keys[i].X + gX, keyboard.keys[i].Y + gY, keyboard.keys[i].W, keyboard.keys[i].H);
			}
		}
		else {	//render text
			if (keyboard.keys[i].text.length > 0){
				ctx.textAlign = "center";
				ctx.font = Math.round(keyboard.keys[i].H).toString() + "px sans-serif";
				ctx.fillStyle = keyboard.keys[i].RGBA;
				ctx.fillText(keyboard.keys[i].text, keyboard.keys[i].X + (keyboard.keys[i].W/2) + gX, keyboard.keys[i].Y + (keyboard.keys[i].H * 0.75) + gY, 
									keyboard.keys[i].W);
			}
			else{
				ctx.strokeStyle = "red";
				ctx.lineWidth = 5;
				ctx.strokeRect(keyboard.keys[i].X + gX, keyboard.keys[i].Y + gY, keyboard.keys[i].W, keyboard.keys[i].H);
			}
		}
	}
}

function blobToBase64(blob){
	return new Promise((resolve, reject) => {
		const reader = new FileReader();
		reader.onloadend = () => resolve(reader.result);
		reader.onerror = reject;
		reader.readAsDataURL(blob);
	});
}

function importProfile(){	//import keyboard profile from layout profile zip file
	document.getElementById("zipInput").click();
}
document.getElementById("zipInput").addEventListener("change", async function (e) {
	const file = e.target.files[0];
	if (!file) return;	//no files found
	
	//else clear all data:
	newKeyboard("tmp");
	
	const zip = await JSZip.loadAsync(file);
	
	//restore keyboard from JSON
	const jsonFile = zip.file("layout.prof");
	if (jsonFile){
		const jsonText = await jsonFile.async("string");
		keyboard = JSON.parse(jsonText);
		//reunicodize the unicode strings:
		for (let i = 0; i < keyboard.keys.length; i++){
			if (keyboard.keys[i].outputType == setOutputType(1)){	//unicode
				keyboard.keys[i].outputValue = getUnicodeString(keyboard.keys[i].outputValue);
			}
		}
	}
	
	//restore images
	let numberOfFilesToGo = Object.keys(zip.files).length;
	let shaderMatch = new Array();
	Object.keys(zip.files).forEach(async (filename) => {
		console.log(filename);
		if (filename.startsWith("images/") && filename != "images/"){
			const blob = await zip.file(filename).async("blob");
			const url = await blobToBase64(blob);
			addImage(filename.substring(7), url);
		}
		if (filename.startsWith("fonts/") && filename != "fonts/"){
			const blob = await zip.file(filename).async("blob");
			const url = await blobToBase64(blob);
			addFont(filename.substring(6), url);
		}
		if (filename.startsWith("plugins/") && filename != "plugins/"){
			const blob = await zip.file(filename).async("blob");
			const url = await blobToBase64(blob);
			addPlugin(filename.substring(8), url);
		}
		if (filename.startsWith("shaders/") && filename != "shaders/"){
			const blob = await zip.file(filename).async("blob");
			const url = await blobToBase64(blob);
			let matchFound = false;
			for (let i = 0; i < shaderMatch.length; i++){
				if (shaderMatch[i].name.slice(0, -5) == filename.substring(8).slice(0, -5)){
					let shader2 = { name: filename.substring(8), dataURL: url };					
					addShader(filename.substring(8).slice(0, -5), shaderMatch[i], shader2);
					matchFound = true;
					break;
				}
			}
			if (!matchFound){
				shaderMatch.push({
					name: filename.substring(8),
					dataURL: url
				});
			}
		}
		
		//count down
		if (--numberOfFilesToGo == 0){	//all loaded, go render
			//reorder fontList
			let reorderedFontList = new Array();
			for (let i = 0; i < keyboard.fonts.length; i++){
				for (let j = 0; j < fontList.length; j++){
					if (keyboard.fonts[i] == fontList[j].name){
						reorderedFontList.push(fontList[j]);
					}
				}
			}
			fontList = reorderedFontList;
			//add to doc:
			for (let i = 0; i < fontList.length; i++){			
				const p = document.createElement("p");
				p.innerHTML = fontList[i].name;
				document.getElementById("fontLibrary").append(p);				
				//add to fontPicker
				const option = document.createElement("option");
				option.value = fontList[i].name;
				option.text = fontList[i].name;
				document.getElementById("fontPicker").append(option);
			}
			
			//reorder pluginList
			let reorderedPluginList = new Array();
			if (!keyboard.plugins){	//old file formats may not have
				keyboard.plugins = new Array();
			}
			for (let i = 0; i < keyboard.plugins.length; i++){
				for (let j = 0; j < pluginList.length; j++){
					if (keyboard.plugins[i] == pluginList[j].name){
						reorderedPluginList.push(pluginList[j]);
					}
				}
			}
			pluginList = reorderedPluginList;
			//add to doc:
			for (let i = 0; i < pluginList.length; i++){			
				const p = document.createElement("p");
				p.innerHTML = pluginList[i].name;
				document.getElementById("pluginLibrary").append(p);				
				//add to fontPicker
				const option = document.createElement("option");
				option.value = pluginList[i].name;
				option.text = pluginList[i].name;
				document.getElementById("pluginPicker").append(option);
			}
			
			//reorder shaderList
			let reorderedShaderList = new Array();
			let reorderedShaderFiles = new Array();
			for (let i = 0; i < keyboard.shaders.length; i++){
				for (let j = 0; j < shaderList.length; j++){
					if (keyboard.shaders[i] == shaderList[j]){
						reorderedShaderList.push(shaderList[j]);
						reorderedShaderFiles.push(shaderFileList[j]);
					}
				}
			}
			shaderList = reorderedShaderList;
			shaderFileList = reorderedShaderFiles;
			
			for (let i = 0; i < shaderList.length; i++){
				//add p element to library 
				const p = document.createElement("p");
				p.innerHTML = shaderList[i];
				document.getElementById("shaderLibrary").append(p);					
				//add to shader elements:
				const option = document.createElement("option");
				option.value = shaderList[i];
				option.text = shaderList[i];
				document.getElementById("shaderPicker").append(option.cloneNode(true));
				document.getElementById("pressShaderPicker").append(option.cloneNode(true));
				document.getElementById("shaderPickerGlobal").append(option.cloneNode(true));
				document.getElementById("pressShaderPickerGlobal").append(option.cloneNode(true));
				for (let k = 0; k < modShaderElements.length; k++){
					modShaderElements[k].append(option.cloneNode(true));
				}
			}
			
			//checking for plugin event for new file format:
			for (let i = 0; i < keyboard.keys.length; i++){
				if (!keyboard.keys[i].pluginEvent)
					keyboard.keys[i].pluginEvent = -1;
			}
			
			openGlobalParameters();
			renderKeys();
		}
	});
});

function exportProfile(){	//export keyboard profile to zip file
	let imageFolderPath = "images";
	let fontFolderPath = "fonts";
	let shaderFolderPath = "shaders";
	let pluginFolderPath = "plugins";
	
	keyboard.shaders = new Array();
	keyboard.fonts = new Array();
	keyboard.plugins = new Array();
	
	const zip = new JSZip();
	
	//add images to ./images/
	const imageFolder = zip.folder(imageFolderPath);
	for (let i = 0; i < imageList.length; i++){
		let base64Data = imageList[i].dataURL.split(",")[1];	//strip data prefix
		console.log(imageList[i].name);
		imageFolder.file(imageList[i].name, base64Data, { base64: true });
	}
	//add fonts to ./fonts/
	const fontFolder = zip.folder(fontFolderPath);
	for (let i = 0; i < fontList.length; i++){
		keyboard.fonts.push(fontList[i].name);
		let base64Data = fontList[i].dataURL.split(",")[1];	//strip prefix
		console.log(fontList[i].name);
		fontFolder.file(fontList[i].name, base64Data, { base64: true });
	}
	//add plugins to ./plugins/
	const pluginFolder = zip.folder(pluginFolderPath);
	for (let i = 0; i < pluginList.length; i++){
		keyboard.plugins.push(pluginList[i].name);
		let base64Data = pluginList[i].dataURL.split(",")[1];	//strip prefix
		console.log(pluginList[i].name);
		pluginFolder.file(pluginList[i].name, base64Data, { base64: true });
	}
	
	
	//add shaders to ./shaders/
	const shaderFolder = zip.folder(shaderFolderPath);
	for (let i = 0; i < shaderFileList.length; i++){
		keyboard.shaders.push(shaderList[i]);
		let base64Data = shaderFileList[i][0].dataURL.split(",")[1];	//strip data prefix
		console.log(shaderFileList[i][0].name);
		shaderFolder.file(shaderFileList[i][0].name, base64Data, { base64: true });
		base64Data = shaderFileList[i][1].dataURL.split(",")[1];	//strip data prefix
		console.log(shaderFileList[i][1].name);
		shaderFolder.file(shaderFileList[i][1].name, base64Data, { base64: true });
	}
	
	//convert unicode output characters into code points:
	
	for (let i = 0; i < keyboard.keys.length; i++){
		if (keyboard.keys[i].outputType == setOutputType(1))
			keyboard.keys[i].outputValue = unicodeize(keyboard.keys[i].outputValue);
	}
	
	//add json layout.prof file
	let exportData = JSON.stringify(keyboard);
	zip.file("layout.prof", exportData);
	
	//reunicodeize
	for (let i = 0; i < keyboard.keys.length; i++){
		if (keyboard.keys[i].outputType == setOutputType(1))
			keyboard.keys[i].outputValue = getUnicodeString(keyboard.keys[i].outputValue);
	}
	
	//download
	zip.generateAsync({type: "blob" }).then(content => {
		const a = document.createElement("a");
		a.href = URL.createObjectURL(content);
		a.download = keyboard.layoutName + ".zip";
		a.click();
		});
}

function getOutputType(outputType){
	if (outputType == "HID"){ return 0; }
	if (outputType == "Unicode"){ return 1; }
	if (outputType == "ProfSwap"){ return 2; }
	return 0;
}

function setOutputType(index){
	if (index == 0){ return "HID"; }
	if (index == 1){ return "Unicode"; }
	if (index == 2){ return "ProfSwap"; }
	return 0;
}

function colorJSformat(hex){	//hex = #RRGGBBAA
	if (!/#([0-9a-fA-F]{8})$/.test(hex)){
		throw new Error("Invalid RGBA string");
	}
	if (hex.substr(0, 1) == "#")
		hex = hex.substr(1);
	let r = parseInt(hex.slice(0, 2), 16);
	let g = parseInt(hex.slice(2, 4), 16);
	let b = parseInt(hex.slice(4, 6), 16);
	let a = parseInt(hex.slice(6, 8), 16) / 255;
	
	return ("rgba(" + r.toString() + "," + g.toString() + "," + b.toString() + "," + a.toString() + ")");
}

function openGlobalParameters(){
	globalParamsOpen = true;
	closeKey();
	document.getElementById("globalElements").style.display = "block";
	//set parameters:
	/*
		this.layoutName = layoutName;
		this.globalOffsetX = 0;
		this.globalOffsetY = 0;
		this.backgroundImgPath = "";
		this.backgroundIsVideo = false;
		this.backgroundColor = "#FF00FFFF";
		
		this.modiferKeyIndex = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];	//l-[Shift,Ctrl,Alt,UI]R-[],Caps,Fn
	*/
	document.getElementById("profileNameInput").value = keyboard.layoutName;
	document.getElementById("globalXInput").value = keyboard.globalOffsetX.toString();
	document.getElementById("globalYInput").value = keyboard.globalOffsetY.toString();
	document.getElementById("globalBGSrcInput").value = keyboard.backgroundImgPath;
	document.getElementById("useVideoInput").checked = keyboard.backgroundIsVideo;
	document.getElementById("bgColorInput").jscolor.fromString(colorJSformat(keyboard.backgroundColor));
	
	let select = document.getElementById("shaderPickerGlobal");
	if (keyboard.shaderGlobal < shaderList.length)
		select.selectedIndex = keyboard.shaderGlobal + 1;	//0 is default
	else {	//shader not found
		keyboard.shaderGlobal = -1;
		select.selectedIndex = keyboard.shaderGlobal + 1;	//0 is default
	}
	
	select = document.getElementById("pressShaderPickerGlobal");
	if (keyboard.pressShaderGlobal < shaderList.length)
		select.selectedIndex = keyboard.pressShaderGlobal + 1;	//0 is default
	else {	//pressShader not found
		keyboard.pressShaderGlobal = -1;
		select.selectedIndex = keyboard.pressShaderGlobal + 1;	//0 is default
	}
	
	//return colors:
	document.getElementById("globalXInput").style.color = "black";
	document.getElementById("globalYInput").style.color = "black";
	
	return;
}

function openKey(k){
	closeGlobalParameters();
	selectedKey = k;
	//enable keyElements
	document.getElementById("keyElements").style.display = "block";
	//add key's parameters onto the area on the right

	document.getElementById("keyIDp").innerHTML = "ID: " + k.toString();

	document.getElementById("zInput").value = keyboard.keys[k].Z.toString();
	document.getElementById("xInput").value = keyboard.keys[k].X.toString();
	document.getElementById("yInput").value = keyboard.keys[k].Y.toString();
	document.getElementById("wInput").value = keyboard.keys[k].W.toString();
	document.getElementById("hInput").value = keyboard.keys[k].H.toString();
	
	document.getElementById("colorInput").jscolor.fromString(colorJSformat(keyboard.keys[k].RGBA));
	document.getElementById("isNotRectInput").checked = !keyboard.keys[k].isRect;
	document.getElementById("useImageInput").checked = keyboard.keys[k].useImage;
	
	let select = document.getElementById("fontPicker");
	select.selectedIndex = keyboard.keys[k].font + 1;	//0 is default
	
	document.getElementById("textInput").value = keyboard.keys[k].text;
	document.getElementById("outputValueInput").value = keyboard.keys[k].outputValue;
	document.getElementById("outputPicker").selectedIndex = getOutputType(keyboard.keys[k].outputType);
	document.getElementById("affectedByCapsLock").checked = keyboard.keys[k].affectedByCapsLock;


	select = document.getElementById("shaderPicker");
	if (keyboard.keys[k].shader < shaderList.length)
		select.selectedIndex = keyboard.keys[k].shader + 1;	//0 is default
	else {	//shader not found
		keyboard.keys[k].shader = -1;
		select.selectedIndex = keyboard.keys[k].shader + 1;	//0 is default
	}
	
	select = document.getElementById("pressShaderPicker");
	if (keyboard.keys[k].pressShader < shaderList.length)
		select.selectedIndex = keyboard.keys[k].pressShader + 1;	//0 is default
	else {	//pressShader not found
		keyboard.keys[k].pressShader = -1;
		select.selectedIndex = keyboard.keys[k].pressShader + 1;	//0 is default
	}
	
	select = document.getElementById("pluginPicker");
	if (keyboard.keys[k].pluginEvent < pluginList.length)
		select.selectedIndex = keyboard.keys[k].pluginEvent + 1;	//0 is default
	else {	//pressShader not found
		keyboard.keys[k].pluginEvent = -1;
		select.selectedIndex = keyboard.keys[k].pluginEvent + 1;	//0 is default
	}
	
	for (let i = 0; i < numberOfModifiers; i++){
		modTypeElements[i].selectedIndex = getOutputType(keyboard.keys[k].modType[i]);
		modValueElements[i].value = keyboard.keys[k].modOutputValue[i];
		modTextElements[i].value = keyboard.keys[k].modText[i];
		if (keyboard.keys[k].modShader[i] < shaderList.length)
			modShaderElements[i].selectedIndex = keyboard.keys[k].modShader[i] + 1;	//0 is default
		else { 	//modShader not found
			keyboard.keys[k].modShader[i] = -1;
			modShaderElements[i].selectedIndex = keyboard.keys[k].modShader[i] + 1;	//0 is default
		}
	}
	
	setFontStatus();
	setRectStatus();
	
	//resetting colors:
	document.getElementById("zInput").style.color = "black";
	document.getElementById("xInput").style.color = "black";
	document.getElementById("yInput").style.color = "black";
	document.getElementById("wInput").style.color = "black";
	document.getElementById("hInput").style.color = "black";
	document.getElementById("textInput").style.color = "black";
}

function closeGlobalParameters(){
	globalParamsOpen = false;
	document.getElementById("globalElements").style.display = "none";
}

function closeKey(){
	selectedKey = -1;
	document.getElementById("keyElements").style.display = "none";
}

function getImageFromName(imageName){
	for (let i = 0; i < imageList.length; i++){
		if (imageName == imageList[i].name)
			return imageList[i].image;
	}
	return null;
}

function setRectStatus(){
	if (selectedKey == -1) return
	if (!keyboard.keys[selectedKey].isRect){	//is texture, enable elements
		document.getElementById("useImageInput").disabled = false;
		document.getElementById("fontPicker").disabled = false;
		document.getElementById("textInput").disabled = false;
		setFontStatus();
	}
	else {	//is rect, disable elements
		document.getElementById("useImageInput").disabled = true;
		document.getElementById("fontPicker").disabled = true;
		document.getElementById("textInput").disabled = true;
	}
}

function setFontStatus(){
	if (selectedKey == -1) return
	//called when things are already enabled
	if (keyboard.keys[selectedKey].useImage){
		document.getElementById("keyText").innerHTML = "Image";
		document.getElementById("modKeyTextLabel").innerHTML = "Modifier Key Image";
		document.getElementById("fontPicker").disabled = true;
	}
	else {	//use text
		document.getElementById("keyText").innerHTML = "Label";
		document.getElementById("modKeyTextLabel").innerHTML = "Modifier Key Label";
		document.getElementById("fontPicker").disabled = false;
	}
}

document.getElementById("isNotRectInput").addEventListener('input', function(e){
	if (selectedKey == -1) return
	keyboard.keys[selectedKey].isRect = !e.target.checked;
	setRectStatus();
	renderKeys();
});

document.getElementById("useImageInput").addEventListener('input', function(e){
	if (selectedKey == -1) return
	keyboard.keys[selectedKey].useImage = e.target.checked;
	setFontStatus();
	renderKeys();
});

document.getElementById("massImageButton").addEventListener("click", (e) => {
	document.getElementById("massImageInput").click();
});

document.getElementById("massFontButton").addEventListener("click", (e) => {
	document.getElementById("massFontInput").click();
});

document.getElementById("massShaderButton").addEventListener("click", (e) => {
	document.getElementById("massShaderInput").click();
});

document.getElementById("massPluginButton").addEventListener("click", (e) => {
	document.getElementById("massPluginInput").click();
});

function addImage(imgName, dataURL){
	//add to image library element
	const img = document.createElement("img");
	img.src = dataURL;
	img.title = imgName;
	img.addEventListener("click", (e) => {
		if (globalParamsOpen){
			keyboard.backgroundImgPath = e.target.title;		
			document.getElementById("globalBGSrcInput").value = e.target.title;			
			document.getElementById("globalBGSrcInput").style.color = "black";
			renderKeys();
			return;
		}
		if (selectedKey == -1) { return; }
		else if (modImageChangeInTime){
			modImageChangeInTime = false;
			keyboard.keys[selectedKey].modText[modImageFocus] = e.target.title;		
			modTextElements[modImageFocus].value = e.target.title;
			modTextElements[modImageFocus].style.color = "black";
			renderKeys();
			return;
		}
		else if (keyboard.keys[selectedKey].useImage == false){ 
			document.getElementById("isNotRectInput").checked = true;
			document.getElementById("isNotRectInput").dispatchEvent(new Event("input"));
			document.getElementById("useImageInput").checked = true; 
			document.getElementById("useImageInput").dispatchEvent(new Event("input"));
		}
		keyboard.keys[selectedKey].text = e.target.title;		
		document.getElementById("textInput").value = e.target.title;
		document.getElementById("textInput").style.color = "black";
		renderKeys();
	});
	imageList.push({
		name: imgName,
		dataURL: dataURL,
		image: img
	});
	//reverse order add to library:
	//document.getElementById("imageLibrary").insertBefore(img, document.getElementById("imageLibrary").children[1]);
	document.getElementById("imageLibrary").append(img);
	return;
}

function addFont(fontName, dataURL){
	fontList.push({
		name: fontName,
		dataURL: dataURL
	});
	return;
}

function addShader(shaderName, shaderFile1, shaderFile2){
	shaderList.push(shaderName);
	shaderFileList.push([shaderFile1, shaderFile2]);
	return;
}

function addPlugin(pluginName, dataURL){
	pluginList.push({
		name: pluginName,
		dataURL: dataURL
	});
	return;
}

document.getElementById("massImageInput").addEventListener("change", (e) => {
	let files = e.target.files;
	//add images to list of images
	for (let file of files){
		let reader = new FileReader();
		reader.onload = function(f) {
			let dontAdd = false;
			for (let i = 0; i < imageList.length; i++){
				if (imageList[i].dataURL == f.target.result){
					dontAdd = true;
					break;
				}
			}
			if (!dontAdd){
				addImage(file.name, f.target.result);
			}
		};
		reader.readAsDataURL(file);
	}
	if (files.length > 0){
		document.getElementById("imageLibButton").click();
	}
});

document.getElementById("massFontInput").addEventListener("change", (e) => {
	let files = e.target.files;
	//add images to list of images
	for (let file of files){
		let reader = new FileReader();
		reader.onload = function(f) {
			let dontAdd = false;
			for (let i = 0; i < fontList.length; i++){
				if (fontList[i].dataURL == f.target.result){
					dontAdd = true;
					break;
				}
			}
			if (!dontAdd){
				addFont(file.name, f.target.result);
				//add to p library element
				const p = document.createElement("p");
				p.innerHTML = file.name;
				document.getElementById("fontLibrary").append(p);
				
				//add to fontPicker
				const option = document.createElement("option");
				option.value = file.name;
				option.text = file.name;
				document.getElementById("fontPicker").append(option);
			}
		};
		reader.readAsDataURL(file);
	}
	if (files.length > 0){
		document.getElementById("fontsLibButton").click();
	}
});

document.getElementById("massPluginInput").addEventListener("change", (e) => {
	let files = e.target.files;
	//add images to list of images
	for (let file of files){
		let reader = new FileReader();
		reader.onload = function(f) {
			let dontAdd = false;
			for (let i = 0; i < pluginList.length; i++){
				if (pluginList[i].dataURL == f.target.result){
					dontAdd = true;
					break;
				}
				else{
					console.log("Identical Plugin detected: " + pluginList[i].name + " == " + file.name);
				}
			}
			if (!dontAdd){
				addPlugin(file.name, f.target.result);
				//add to p library element
				const p = document.createElement("p");
				p.innerHTML = file.name;
				document.getElementById("pluginLibrary").append(p);
				
				//add to fontPicker
				const option = document.createElement("option");
				option.value = file.name;
				option.text = file.name;
				document.getElementById("pluginPicker").append(option);
			}
		};
		reader.readAsDataURL(file);
	}
	if (files.length > 0){
		document.getElementById("pluginLibButton").click();
	}
});

document.getElementById("massShaderInput").addEventListener("change", (e) => {
	let files = e.target.files;
	//add images to list of images
	
	let fragFiles = new Array();
	let vertFiles = new Array();
	
	let allLoaded = files.length;
	
	for (let file of files){
		let reader = new FileReader();
		reader.onload = function(f) {
			//sends frags and verts to different lists
			if (file.name.endsWith(".frag")){
				fragFiles.push({
					name: file.name,
					type: file.type,
					size: file.size,
					dataURL: f.target.result
				});
			}
			else if (file.name.endsWith(".vert")){
				vertFiles.push({
					name: file.name,
					type: file.type,
					size: file.size,
					dataURL: f.target.result
				});
			}
			allLoaded--;
		};
		reader.readAsDataURL(file);
	}
	
	const waitForAll = setInterval(() => { while (allLoaded > 0){ continue; } 
	
	clearInterval(waitForAll);
	//check to see if there is both a .frag and a .vert
	for (let i = 0; i < fragFiles.length; i++){
		for (let j = 0; j < vertFiles.length; j++){
		if (i == -1 || j == -1) continue;
			if (fragFiles[i].name.slice(0, -5) == vertFiles[j].name.slice(0, -5)){
				let dontAdd = false;
				for (let k = 0; k < shaderList.length; k++){
					if (shaderList[k] == fragFiles[i].name.slice(0, -5)){
						dontAdd = true;
						alert("Repeat Shader Name: " + fragFiles[i].name.slice(0, -5));
						fragFiles.splice(i--, 1);
						vertFiles.splice(j--, 1);
						k = shaderList.length;
					}
				}
				if (!dontAdd){
					addShader(fragFiles[i].name.slice(0, -5), fragFiles[i], vertFiles[i]);
					//add p element to library 
					const p = document.createElement("p");
					p.innerHTML = fragFiles[i].name.slice(0, -5);
					document.getElementById("shaderLibrary").append(p);
						
					//add to shader elements:
					const option = document.createElement("option");
					option.value = fragFiles[i].name.slice(0, -5);
					option.text = fragFiles[i].name.slice(0, -5);
					document.getElementById("shaderPicker").append(option.cloneNode(true));
					document.getElementById("pressShaderPicker").append(option.cloneNode(true));
					document.getElementById("shaderPickerGlobal").append(option.cloneNode(true));
					document.getElementById("pressShaderPickerGlobal").append(option.cloneNode(true));
					for (let k = 0; k < modShaderElements.length; k++){
						modShaderElements[k].append(option.cloneNode(true));
					}
	
					fragFiles.splice(i--, 1);
					vertFiles.splice(j--, 1);
				}
			}
		}
	}
	if (fragFiles.length != 0 || vertFiles.length != 0){
		alert("Some shaders could not be imported due to missing .vert or .frag. Shader program files should share same file name.");
	}
	document.getElementById("shaderLibButton").click();
	}, 50);
});

//parameter changing events-----------------------------------------------------------------------------------------------
const maxZ = 4;
const maxX = 10000;
const maxY = 5000;
const maxSize = 4000;
document.getElementById("zInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxZ){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.keys[selectedKey].Z = newVal;
	renderKeys();
});

document.getElementById("xInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxX){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.keys[selectedKey].X = newVal;
	renderKeys();
});

document.getElementById("yInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxY){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.keys[selectedKey].Y = newVal;
	renderKeys();
});

document.getElementById("wInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxSize){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.keys[selectedKey].W = newVal;
	renderKeys();
});

document.getElementById("hInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxSize){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.keys[selectedKey].H = newVal;
	renderKeys();
});

document.getElementById("colorInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	let regex = /rgba\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*([\d.]+)\s*\)/;
	let colors = e.target.value.match(regex);	
	//proper formatting should be handled by color-js
	//convert to hex string
	let r = parseInt(colors[1], 10);
	let g = parseInt(colors[2], 10);
	let b = parseInt(colors[3], 10);
	let a = parseFloat(colors[4]);
	let colorString = "#" + r.toString(16).padStart(2, '0') + g.toString(16).padStart(2, '0') + b.toString(16).padStart(2, '0') + 
						Math.round(a * 255).toString(16).padStart(2, '0');
	//save new value to selected key:
	keyboard.keys[selectedKey].RGBA = colorString.toUpperCase();
	renderKeys();
});

document.getElementById("isNotRectInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].isRect = !e.target.checked;
});

document.getElementById("useImageInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].useImage = e.target.checked;
});

document.getElementById("fontPicker").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].font = e.target.selectedIndex - 1;
});

document.getElementById("pluginPicker").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].pluginEvent = e.target.selectedIndex - 1;
});

document.getElementById("shaderPicker").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].shader = e.target.selectedIndex - 1;
});

document.getElementById("pressShaderPicker").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].pressShader = e.target.selectedIndex - 1;
});

document.getElementById("shaderPickerGlobal").addEventListener("change", (e) => {
	if (selectedKey != -1) { return; }
	keyboard.shaderGlobal = e.target.selectedIndex - 1;
});

document.getElementById("pressShaderPickerGlobal").addEventListener("change", (e) => {
	if (selectedKey != -1) { return; }
	keyboard.pressShaderGlobal = e.target.selectedIndex - 1;
});

document.getElementById("textInput").addEventListener("change", (e) => {
	if (selectedKey == -1) { return; }
	//check if is in imageList
	let notFound = true;
	if (keyboard.keys[selectedKey].useImage){
		for (let i = 0; i < imageList.length; i++){
			if (e.target.value == imageList[i].name){
				notFound = false;
				break;
			}
		}
		if (notFound){
			e.target.style.color = "red";
			return;
		}
		else e.target.style.color = "black";
	}
	//save new value to selected key:
	keyboard.keys[selectedKey].text = e.target.value;
	renderKeys();
});

document.getElementById("outputPicker").addEventListener("change", (e) => {
	if (selectedKey == -1){ return; }
	keyboard.keys[selectedKey].outputType = setOutputType(e.target.selectedIndex);
});

document.getElementById("outputValueInput").addEventListener("change", (e) => {
	if (selectedKey == -1){ return; }
	keyboard.keys[selectedKey].outputValue = e.target.value;
});

function getUnicodeString(s){
	let codePoint = parseInt(s, 16);
	if (isNaN(codePoint))
		return s;
	let unicodeString = String.fromCodePoint(codePoint);
	return unicodeString;
}

function unicodeize(s){	//expecting input of type string
	if (!(typeof s === 'string' && [...s].length > 0)){
		console.log('not unicode string');
		return;
	}
	let val = [...s].map(char => { const codePoint = char.codePointAt(0);
		return codePoint.toString(16).padStart(4, '0'); }).join(' ');
	console.log(val);
	return val;
}

document.getElementById("affectedByCapsLock").addEventListener("change", (e) => {
	if (selectedKey == -1){ return; }
	keyboard.keys[selectedKey].affectedByCapsLock = e.target.checked;
});

function changeModType(e, i){
		if (selectedKey == -1){ return; }
		keyboard.keys[selectedKey].modType[i] = setOutputType(e.target.selectedIndex);		
}

function changeModOutput(e, i){
	if (selectedKey == -1){ return; }
	keyboard.keys[selectedKey].modOutputValue[i] = e.target.value;
}

function changeModText(e, i){
	if (selectedKey == -1){ return; }
	keyboard.keys[selectedKey].modText[i] = e.target.value;
}

function changeModShader(e, i){
	if (selectedKey == -1) { return; }
	keyboard.keys[selectedKey].modShader[i] = e.target.selectedIndex - 1;
}

var modImageFocus = 0;
var modImageChangeInTime = false;
function modImgFocus(e, i){
	if (selectedKey == -1) { return; }
	modImageFocus = i;
}
function modImgBlur(e, i){
	if (selectedKey == -1) { return; }
	modImageChangeInTime = true;
	setTimeout( () => { modImageChangeInTime = false; }, 1000);	//1 second to change image after blur
	//this works because the blur occurs right before the image click, so selection is still active, but only long enough to matter
}

function addModEvents(){
	modTypeElements.push(document.getElementById("modTypeShift"));
	document.getElementById("modTypeShift").addEventListener("change", (e) => { changeModType(e, 0); });
	modTypeElements.push(document.getElementById("modTypeCtrl"));
	document.getElementById("modTypeCtrl").addEventListener("change", (e) => { changeModType(e, 1); });
	modTypeElements.push(document.getElementById("modTypeUI"));
	document.getElementById("modTypeUI").addEventListener("change", (e) => { changeModType(e, 2); });
	modTypeElements.push(document.getElementById("modTypeAlt"));
	document.getElementById("modTypeAlt").addEventListener("change", (e) => { changeModType(e, 3); });
	modTypeElements.push(document.getElementById("modTypeFn"));
	document.getElementById("modTypeFn").addEventListener("change", (e) => { changeModType(e, 4); });
	
	modValueElements.push(document.getElementById("modValueShift"));
	document.getElementById("modValueShift").addEventListener("change", (e) => { changeModOutput(e, 0); });
	modValueElements.push(document.getElementById("modValueCtrl"));
	document.getElementById("modValueCtrl").addEventListener("change", (e) => { changeModOutput(e, 1); });
	modValueElements.push(document.getElementById("modValueUI"));
	document.getElementById("modValueUI").addEventListener("change", (e) => { changeModOutput(e, 2); });
	modValueElements.push(document.getElementById("modValueAlt"));
	document.getElementById("modValueAlt").addEventListener("change", (e) => { changeModOutput(e, 3); });
	modValueElements.push(document.getElementById("modValueFn"));
	document.getElementById("modValueFn").addEventListener("change", (e) => { changeModOutput(e, 4); });
	
	modTextElements.push(document.getElementById("modTextShift"));
	document.getElementById("modTextShift").addEventListener("change", (e) => { changeModText(e, 0); });
	document.getElementById("modTextShift").addEventListener("focus", (e) => { modImgFocus(e, 0); });
	document.getElementById("modTextShift").addEventListener("blur", (e) => { modImgBlur(e, 0); });
	modTextElements.push(document.getElementById("modTextCtrl"));
	document.getElementById("modTextCtrl").addEventListener("change", (e) => { changeModText(e, 1); });
	document.getElementById("modTextCtrl").addEventListener("focus", (e) => { modImgFocus(e, 1); });
	document.getElementById("modTextCtrl").addEventListener("blur", (e) => { modImgBlur(e, 1); });
	modTextElements.push(document.getElementById("modTextUI"));
	document.getElementById("modTextUI").addEventListener("change", (e) => { changeModText(e, 2); });
	document.getElementById("modTextUI").addEventListener("focus", (e) => { modImgFocus(e, 2); });
	document.getElementById("modTextUI").addEventListener("blur", (e) => { modImgBlur(e, 2); });
	modTextElements.push(document.getElementById("modTextAlt"));
	document.getElementById("modTextAlt").addEventListener("change", (e) => { changeModText(e, 3); });
	document.getElementById("modTextAlt").addEventListener("focus", (e) => { modImgFocus(e, 3); });
	document.getElementById("modTextAlt").addEventListener("blur", (e) => { modImgBlur(e, 3); });
	modTextElements.push(document.getElementById("modTextFn"));
	document.getElementById("modTextFn").addEventListener("change", (e) => { changeModText(e, 4); });
	document.getElementById("modTextFn").addEventListener("focus", (e) => { modImgFocus(e, 4); });
	document.getElementById("modTextFn").addEventListener("blur", (e) => { modImgBlur(e, 4); });
	
	modShaderElements.push(document.getElementById("modShaderShift"));
	document.getElementById("modShaderShift").addEventListener("change", (e) => { changeModShader(e, 0); });
	modShaderElements.push(document.getElementById("modShaderCtrl"));
	document.getElementById("modShaderCtrl").addEventListener("change", (e) => { changeModShader(e, 1); });
	modShaderElements.push(document.getElementById("modShaderUI"));
	document.getElementById("modShaderUI").addEventListener("change", (e) => { changeModShader(e, 2); });
	modShaderElements.push(document.getElementById("modShaderAlt"));
	document.getElementById("modShaderAlt").addEventListener("change", (e) => { changeModShader(e, 3); });
	modShaderElements.push(document.getElementById("modShaderFn"));
	document.getElementById("modShaderFn").addEventListener("change", (e) => { changeModShader(e, 4); });
	
	let codeList = ["FN","NULL","TRANSPARENT","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R",
			"S","T","U","V","W","X","Y","Z","1","2","3","4","5","6","7","8","9","0","ENTER","ESCAPE","BACKSPACE",
			"TAB","SPACE","MINUS","EQUAL","LEFT_BRACKET","RIGHT_BRACKET","BACKSLASH","NONUS_HASH","SEMICOLON",
			"QUOTE","GRAVE","COMMA","DOT","SLASH","CAPS_LOCK","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
			"PRINT_SCREEN","SCROLL_LOCK","PAUSE","INSERT","HOME","PAGE_UP","DELETE","END","PAGE_DOWN","RIGHT","LEFT","DOWN","UP",
			"NUM_LOCK","KP_SLASH","KP_ASTERISK","KP_MINUS","KP_PLUS","KP_ENTER","KP_1","KP_2","KP_3","KP_4","KP_5","KP_6","KP_7",
			"KP_8","KP_9","KP_0","KP_DOT","NONUS_BACKSLASH","KC_APPLICATION","KB_POWER","KP_EQUAL","F13","F14","F15","F16","F17",
			"F18","F19","F20","F21","F22","F23","F24","EXECUTE","HELP","MENU","SELECT","STOP","AGAIN","UNDO","CUT","COPY","PASTE",
			"FIND","KB_MUTE","KB_VOLUME_UP","KB_VOLUME_DOWN","LOCKING_CAPS_LOCK","LOCKING_NUM_LOCK","LOCKING_SCROLL_LOCK",
			"KP_COMMA","KP_EQUAL","INTERNATIONAL_1","INTERNATIONAL_2","INTERNATIONAL_3","INTERNATIONAL_4","INTERNATIONAL_5",
			"INTERNATIONAL_6","INTERNATIONAL_7","INTERNATIONAL_8","INTERNATIONAL_9","LANGUAGE_1","LANGUAGE_2","LANGUAGE_3",
			"LANGUAGE_4","LANGUAGE_5","LANGUAGE_6","LANGUAGE_7","LANGUAGE_8","LANGUAGE_9","ALTERNATE_ERASE","SYSTEM_REQUEST",
			"CANCEL","CLEAR","PRIOR","RETURN","SEPARATOR","OUT","OPER","CLEAR_AGAIN","CRSEL","EXSEL","SYSTEM_POWER","SYSTEM_SLEEP",
			"SYSTEM_WAKE","AUDIO_MUTE","AUDIO_VOL_UP","AUDIO_VOL_DOWN","MEDIA_NEXT_TRACK","MEDIA_PREV_TRACK","MEDIA_STOP",
			"MEDIA_PLAY_PAUSE","MEDIA_SELECT","MEDIA_EJECT","MAIL","CALCULATOR","MY_COMPUTER","WWW_SEARCH","WWW_HOME","WWW_BACK",
			"WWW_FORWARD","WWW_STOP","WWW_REFRESH","WWW_FAVORITES","MEDIA_FAST_FORWARD","MEDIA_REWIND","BRIGHTNESS_UP","BRIGHTNESS_DOWN",
			"CONTROL_PANEL","ASSISTANT","MISSION_CONTROL","LAUNCHPAD","MOUSE_CURSOR_UP","MOUSE_CURSOR_DOWN","MOUSE_CURSOR_LEFT",
			"MOUSE_CURSOR_RIGHT","MOUSE_BUTTON_1","MOUSE_BUTTON_2","MOUSE_BUTTON_3","MOUSE_BUTTON_4","MOUSE_BUTTON_5",
			"MOUSE_BUTTON_6","MOUSE_BUTTON_7","MOUSE_BUTTON_8","MOUSE_WHEEL_UP","MOUSE_WHEEL_DOWN","MOUSE_WHEEL_LEFT",
			"MOUSE_WHEEL_RIGHT","MOUSE_ACCELERATION_0","MOUSE_ACCELERATION_1","MOUSE_ACCELERATION_2",
			"LEFT_CTRL","LEFT_SHIFT","LEFT_ALT","LEFT_GUI","RIGHT_CTRL","RIGHT_SHIFT","RIGHT_ALT","RIGHT_GUI"];
	let dataList = document.getElementById("HIDcodes");
	let dataModList = [document.getElementById("HIDcodes1"), document.getElementById("HIDcodes2"), 
	document.getElementById("HIDcodes3"), document.getElementById("HIDcodes4"), document.getElementById("HIDcodes5")];
	codeList.forEach(item => {
		const option = document.createElement("option");
		option.value = item;
		dataList.appendChild(option);
		for (let k = 0; k < dataModList.length; k++){
			dataModList[k].append(option.cloneNode(true));
		}
	});
}

document.getElementById("profileNameInput").addEventListener("change", (e) => {
console.log(globalParamsOpen);
	if (!globalParamsOpen) { return; }
	if (e.target.value.length > 0)
		keyboard.layoutName = e.target.value;
});
document.getElementById("globalXInput").addEventListener("change", (e) => {
	if (!globalParamsOpen){ return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxSize){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.globalOffsetX = newVal;
	renderKeys();
});
document.getElementById("globalYInput").addEventListener("change", (e) => {
	if (!globalParamsOpen){ return; }
	//get and change to int
	let newVal = parseInt(e.target.value);
	//check if is correct format
	if (isNaN(newVal) || newVal < 0 || newVal > maxSize){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value to selected key:
	keyboard.globalOffsetY = newVal;
	renderKeys();
});
document.getElementById("bgColorInput").addEventListener("change", (e) => {
	if (!globalParamsOpen){ return; }
	let regex = /rgba\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*([\d.]+)\s*\)/;
	let colors = e.target.value.match(regex);	
	//proper formatting should be handled by color-js
	//convert to hex string
	let r = parseInt(colors[1], 10);
	let g = parseInt(colors[2], 10);
	let b = parseInt(colors[3], 10);
	let a = parseFloat(colors[4]);
	let colorString = "#" + r.toString(16).padStart(2, '0') + g.toString(16).padStart(2, '0') + b.toString(16).padStart(2, '0') + 
						Math.round(a * 255).toString(16).padStart(2, '0');
	//save new value to selected key:
	keyboard.backgroundColor = colorString.toUpperCase();
	renderKeys();
});
document.getElementById("globalBGSrcInput").addEventListener("change", (e) => {
	if (!globalParamsOpen){ return; }
	if (e.target.value == ""){
		keyboard.backgroundImgPath = "";
		renderKeys();
		return;
	}
	//check if is in imageList
	let notFound = true;
	for (let i = 0; i < imageList.length; i++){
		if (e.target.value == imageList[i].name){
			notFound = false;
			break;
		}
	}
	if (notFound){
		e.target.style.color = "red";
		return;
	}
	else e.target.style.color = "black";
	//save new value as background image
	keyboard.backgroundImgPath = e.target.value;
	renderKeys();
});
document.getElementById("useVideoInput").addEventListener("change", (e) => {
	if (!globalParamsOpen){ return; }
	keyboard.backgroundIsVideo = e.target.checked;
});

var setModKeyOnNextClick = false;
var modKeyIndex = -1;
function setModKey(index){
	setModKeyOnNextClick = true;
	modKeyIndex = index;
	canvas.style.border = "5px solid red";
}

canvas.addEventListener('click', canvasClick);

function canvasClick(e){
	//console.log(e);
	var rect = canvas.getBoundingClientRect();
	let x = event.clientX - rect.left;
	let y = event.clientY - rect.top;
	let scaleX = canvas.width / rect.width;
	let scaleY = canvas.height / rect.height;
	x *= scaleX;
	y *= scaleY;
	//console.log(x + ", " + y);
	let openedKey = false;
	for (let i = 0; i < keyboard.keys.length; i++){
		if (x >= keyboard.keys[i].X && x <= keyboard.keys[i].X + keyboard.keys[i].W &&
			y >= keyboard.keys[i].Y && y <= keyboard.keys[i].Y + keyboard.keys[i].H){	
			openKey(i);	
			openedKey = true;
			break;
		}
	}	
	if (!openedKey){
		openGlobalParameters();
	}
}

function showTab(index){
	let pages = document.querySelectorAll('.tab-page');
	pages.forEach((page, i) => {
		page.classList.toggle('active', i == index);
	});
}

document.addEventListener('keydown', keyDown);
document.addEventListener('keyup', keyUp);
var modDown = [false, false, false, false, false];	//shift, ctrl, GUI, alt, Fn
var capsLock = false;
function keyDown(e){
	console.log(e.key);
	if (e.key == "Shift"){
		modDown[0] = true;
		renderKeys();
	}
	if (e.key == "Control"){
		modDown[1] = true;
		renderKeys();
	}
	if (e.key == "ScrollLock"){
		modDown[2] = true;
		renderKeys();
	}
	if (e.key == "Alt"){
		modDown[3] = true;
		renderKeys();
	}
	if (e.key == "Pause"){
		modDown[4] = true;
		renderKeys();
	}
	if (e.key == "CapsLock"){
		if (e.getModifierState("CapsLock")){
			capsLock = true;	
			renderKeys();
		}
		else{
			capsLock = false;
			renderKeys();
		}
	}
	if (e.key == "Escape"){
		openGlobalParameters();
		renderKeys();
	}
	
	if (modDown[1] && e.key == "f"){
		if (canvas.requestFullscreen){ canvas.requestFullscreen(); }
		else if (canvas.webkitRequestFullscreen){ canvas.webkitRequestFullscreen(); }
		else if (canvas.msRequestFullscreen){ canvas.msRequestFullscreen(); }
	}	

}

function keyUp(e){
	if (e.key == "Shift"){
		modDown[0] = false;
		renderKeys();
	}
	if (e.key == "Control"){
		modDown[1] = false;
		renderKeys();
	}
	if (e.key == "ScrollLock"){
		modDown[2] = false;
		renderKeys();
	}
	if (e.key == "Alt"){
		modDown[3] = false;
		renderKeys();
	}
	if (e.key == "Pause"){
		modDown[4] = false;
		renderKeys();
	}
}

//default keyboard layouts------------------------------------------------------------------------------------------------

function defaultZ(){ return 1; }
function defaultKeySize(){ return 175; }
function defaultIsRect(){ return true; }
function defaultColor(){ return "#000000FF"; }
function defaultOutputType(){ return "HID"; }
function defaultOutputValue(){ return "A"; }
function blankModType(){ return ["HID", "HID", "HID", "HID", "HID"]; }
function blankModText(){ return ["", "", "", "", ""]; }
function blankModShader(){ return [-1, -1, -1, -1, -1]; }
function blankOutputValue(){ return ["", "", "", "", ""]; }
function getKeyGap(){ return 13; }

function newKeyboardANSI(askName){
	closeKey();
	closeGlobalParameters();
	console.log('ANSI keyboard');
	if (askName){
		let keyboardName = prompt("New Keyboard Layout Name:");
		if (keyboardName != null && keyboardName.length > 0)
			newKeyboard(keyboardName);
		else
			return;
			
		keyboard.layoutName = keyboardName;
	}
	
	let leftXoffset = 18;
	let topYoffset = 5;
	let keyGap = getKeyGap();
	//first row of keyboard.keys:
	let fnKeyGap = 100;
		keyboard.keys.push(new key(defaultZ(), leftXoffset, topYoffset, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),	//ESC
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
				
	for (let i = 0; i < 4; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + defaultKeySize()*2 + keyGap*2 + (defaultKeySize() + keyGap) * i, topYoffset, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
	for (let i = 0; i < 4; i++){
		keyboard.keys.push(new key(defaultZ(), fnKeyGap + leftXoffset + defaultKeySize()*6 + keyGap*5.5 + (defaultKeySize() + keyGap) * i, topYoffset, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
	for (let i = 0; i < 4; i++){
		keyboard.keys.push(new key(defaultZ(), fnKeyGap*2 + leftXoffset + defaultKeySize()*10 + keyGap*9 + (defaultKeySize() + keyGap) * i, topYoffset, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
	//second row:
	for (let i = 0; i < 13; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + (defaultKeySize() + keyGap) * i, defaultKeySize() + keyGap, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
	let deleteKeyWidth = 365;
		keyboard.keys.push(new key(defaultZ(), leftXoffset + (defaultKeySize() + keyGap) * 13, defaultKeySize() + keyGap, 
		deleteKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	//third row:
	let tabKeyWidth = 268;
		keyboard.keys.push(new key(defaultZ(), leftXoffset, (defaultKeySize() + keyGap) * 2, 
		tabKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	for (let i = 0; i < 12; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + tabKeyWidth + defaultKeySize() * i + keyGap * (i+1), (defaultKeySize() + keyGap) * 2, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
		keyboard.keys.push(new key(defaultZ(), leftXoffset + tabKeyWidth + defaultKeySize()*12 + keyGap*13, (defaultKeySize() + keyGap) * 2, 
		tabKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));

	//fourth row:
	let capsKeyWidth = 320;
	let enterKeyWidth = 404;
		keyboard.keys.push(new key(defaultZ(), leftXoffset, (defaultKeySize() + keyGap) * 3, 
		capsKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	for (let i = 0; i < 11; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + capsKeyWidth + defaultKeySize() * i + keyGap * (i+1), (defaultKeySize() + keyGap) * 3, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
		keyboard.keys.push(new key(defaultZ(), leftXoffset + capsKeyWidth + defaultKeySize()*11 + keyGap*12, (defaultKeySize() + keyGap) * 3, 
		enterKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	
	//fifth row:
	let shiftLKeyWidth = enterKeyWidth;225, 126
	let shiftRKeyWidth = 504;
		keyboard.keys.push(new key(defaultZ(), leftXoffset, (defaultKeySize() + keyGap) * 4, 
		shiftLKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	for (let i = 0; i < 10; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + shiftLKeyWidth + defaultKeySize() * i + keyGap * (i+1), (defaultKeySize() + keyGap) * 4, 
		defaultKeySize(), defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
		keyboard.keys.push(new key(defaultZ(), leftXoffset + shiftLKeyWidth + defaultKeySize()*10 + keyGap*11, (defaultKeySize() + keyGap) * 4, 
		shiftRKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	
	//sixth row:
	let modKeyWidth = 225; 
	let spaceKeyWidth = 1136;
	for (let i = 0; i < 3; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + (modKeyWidth + keyGap) * i, (defaultKeySize() + keyGap) * 5, 
		modKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
		keyboard.keys.push(new key(defaultZ(), leftXoffset + (modKeyWidth + keyGap) * 3, (defaultKeySize() + keyGap) * 5, 
		spaceKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
				
	for (let i = 0; i < 4; i++){
		keyboard.keys.push(new key(defaultZ(), leftXoffset + (modKeyWidth + keyGap) * 3 + spaceKeyWidth + modKeyWidth * i + keyGap * (i+1), (defaultKeySize() + keyGap) * 5, 
		modKeyWidth, defaultKeySize(), defaultIsRect(),
		defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
				-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	}
	
	//key output values:
	//row 1
	let k = 0;
	keyboard.keys[k++].outputValue = "ESCAPE";
	keyboard.keys[k++].outputValue = "F1";
	keyboard.keys[k++].outputValue = "F2";
	keyboard.keys[k++].outputValue = "F3";
	keyboard.keys[k++].outputValue = "F4";
	keyboard.keys[k++].outputValue = "F5";
	keyboard.keys[k++].outputValue = "F6";
	keyboard.keys[k++].outputValue = "F7";
	keyboard.keys[k++].outputValue = "F8";
	keyboard.keys[k++].outputValue = "F9";
	keyboard.keys[k++].outputValue = "F10";
	keyboard.keys[k++].outputValue = "F11";
	keyboard.keys[k++].outputValue = "F12";
	//row 2
	keyboard.keys[k++].outputValue = "GRAVE";
	keyboard.keys[k++].outputValue = "1";
	keyboard.keys[k++].outputValue = "2";
	keyboard.keys[k++].outputValue = "3";
	keyboard.keys[k++].outputValue = "4";
	keyboard.keys[k++].outputValue = "5";
	keyboard.keys[k++].outputValue = "6";
	keyboard.keys[k++].outputValue = "7";
	keyboard.keys[k++].outputValue = "8";
	keyboard.keys[k++].outputValue = "9";
	keyboard.keys[k++].outputValue = "0";
	keyboard.keys[k++].outputValue = "MINUS";
	keyboard.keys[k++].outputValue = "EQUAL";
	keyboard.keys[k++].outputValue = "BACKSPACE";
	//row 3
	keyboard.keys[k++].outputValue = "TAB";
	keyboard.keys[k++].outputValue = "Q";
	keyboard.keys[k++].outputValue = "W";
	keyboard.keys[k++].outputValue = "E";
	keyboard.keys[k++].outputValue = "R";
	keyboard.keys[k++].outputValue = "T";
	keyboard.keys[k++].outputValue = "Y";
	keyboard.keys[k++].outputValue = "U";
	keyboard.keys[k++].outputValue = "I";
	keyboard.keys[k++].outputValue = "O";
	keyboard.keys[k++].outputValue = "P";
	keyboard.keys[k++].outputValue = "LEFT_BRACKET";
	keyboard.keys[k++].outputValue = "RIGHT_BRACKET";
	keyboard.keys[k++].outputValue = "BACKSLASH";
	//row 4
	keyboard.keys[k++].outputValue = "CAPS_LOCK";
	keyboard.keys[k++].outputValue = "A";
	keyboard.keys[k++].outputValue = "S";
	keyboard.keys[k++].outputValue = "D";
	keyboard.keys[k++].outputValue = "F";
	keyboard.keys[k++].outputValue = "G";
	keyboard.keys[k++].outputValue = "H";
	keyboard.keys[k++].outputValue = "J";
	keyboard.keys[k++].outputValue = "K";
	keyboard.keys[k++].outputValue = "L";
	keyboard.keys[k++].outputValue = "SEMICOLON";
	keyboard.keys[k++].outputValue = "QUOTE";
	keyboard.keys[k++].outputValue = "ENTER";
	//row 5
	keyboard.keys[k++].outputValue = "LEFT_SHIFT";
	keyboard.keys[k++].outputValue = "Z";
	keyboard.keys[k++].outputValue = "X";
	keyboard.keys[k++].outputValue = "C";
	keyboard.keys[k++].outputValue = "V";
	keyboard.keys[k++].outputValue = "B";
	keyboard.keys[k++].outputValue = "N";
	keyboard.keys[k++].outputValue = "M";
	keyboard.keys[k++].outputValue = "COMMA";
	keyboard.keys[k++].outputValue = "DOT";
	keyboard.keys[k++].outputValue = "SLASH";
	keyboard.keys[k++].outputValue = "RIGHT_SHIFT";
	//row 6
	keyboard.keys[k++].outputValue = "LEFT_CTRL";
	keyboard.keys[k++].outputValue = "LEFT_GUI";
	keyboard.keys[k++].outputValue = "LEFT_ALT";
	keyboard.keys[k++].outputValue = "SPACE";
	keyboard.keys[k++].outputValue = "RIGHT_ALT";
	keyboard.keys[k++].outputValue = "RIGHT_GUI";
	keyboard.keys[k++].outputValue = "RIGHT_CTRL";
	keyboard.keys[k++].outputValue = "FN";
				
	renderKeys();
	openGlobalParameters();
}

function newKeyboardISO(){
	console.log('ISO keyboard');
	closeKey();
	keyboard.keys = new Array;
	renderKeys();
}

function addMacroPad(){
	console.log("Added Macro Pad");
	let leftHandMacroOffset = 2900;
	let keyGap = getKeyGap();
	let topYoffset = 5;
	for (let j = 0; j < 6; j++){
		for (let i = 0; i < 5; i++){
			keyboard.keys.push(new key(defaultZ(), leftHandMacroOffset + (defaultKeySize() + keyGap) * i, (defaultKeySize() + keyGap) * j + topYoffset, 
			defaultKeySize(), defaultKeySize(), defaultIsRect(),
			defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
					-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
		}
	}
	renderKeys();
}

function addBigPad(){
	console.log("Added Macro Pad");
	let leftHandMacroOffset = 2935;
	let keyGap = getKeyGap();
	let topYoffset = 215;
	let bigKeySize = 285;
	let bigKeyGap = 20;
	for (let j = 0; j < 3; j++){
		for (let i = 0; i < 3; i++){
			keyboard.keys.push(new key(defaultZ(), leftHandMacroOffset + (bigKeySize + bigKeyGap) * i, (bigKeySize + bigKeyGap) * j + topYoffset, 
			bigKeySize, bigKeySize, defaultIsRect(),
			defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
					-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
		}
	}
	keyboard.keys.push(new key(defaultZ(), leftHandMacroOffset - 5, 10, 
			930, defaultKeySize(), defaultIsRect(),
			defaultColor(), defaultOutputType(), defaultOutputValue(), false, false, "", 
					-1, -1, -1, blankModType(), blankModText(), blankModShader(), blankOutputValue()));
	renderKeys();
}

setTimeout(start, 1000);


