function fontSizeAdd(elemId, fontURL) {
  var active = getFontStyle(elemId);
  switch (active) {
    case 'font1' :
      setActiveStyle('font2', elemId, fontURL);
      break;
    case 'font2' :
      setActiveStyle('font3', elemId, fontURL);
      break;
    case 'font3' :
      setActiveStyle('font4', elemId, fontURL);
      break;
    case 'font4' :
      setActiveStyle('font5', elemId, fontURL);
      break;
    case 'font5' :
      break;
    default :
      setActiveStyle('font3', elemId, fontURL);
      break;
  }
}

function fontSizeMinus(elemId, fontURL) {
  var active = getFontStyle(elemId);
  switch (active) {
    case 'font5' :
      setActiveStyle('font4', elemId, fontURL);
      break;
    case 'font4' :
      setActiveStyle('font3', elemId, fontURL);
      break;
    case 'font3' :
      setActiveStyle('font2', elemId, fontURL);
      break;
    case 'font2' :
      setActiveStyle('font1', elemId, fontURL);
      break;
    case 'font1' :
       break;
    default :
      setActiveStyle('font3', elemId, fontURL);
      break;
  }
}

function getFontStyle(elemId) {
  var x = document.getElementById(elemId);
  if(!x) {return 'fontDefault';}
  var z = x.getAttribute(document.all ? "className" : "class");
  return z;
}

function setActiveStyle(fontClass, elemId, fontURL) {
  var x = document.getElementById(elemId);
  if(!x) {return;}
  var z = x.getAttribute(document.all ? "className" : "class");

  if(z != fontClass) {
    var fontId;
    switch (fontClass) {
    case 'font5' :
      fontId = 5;
      break;
    case 'font4' :
      fontId = 4;
      break;
    case 'font3' :
      fontId = 3;
      break;
    case 'font2' :
      fontId = 2;
      break;
    case 'font1' :
      fontId = 1;
      break;
    default :
      fontId = 3;
      break;
    }
    var url = "/" + fontURL + "/fontsize/" + fontId;
    new Ajax.Request(url);
    x.setAttribute((document.all ? "className" : "class"), fontClass);
  }
}
