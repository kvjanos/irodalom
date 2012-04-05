function openNS(url, width, height) {
    if ((navigator.appName == "Microsoft Internet Explorer") &&
        (parseFloat(navigator.appVersion) < 4 ))
    {
        return false;
    }
     
    if (!width) var width = 600;
    if (!height) var height = 400;

    var newX=width,newY=height,xOffset=10,yOffset=10;
    var parms = 'width=' + newX +
            ',height=' + newY +
            ',screenX='+ xOffset +
            ',screenY=' + yOffset +
            ',status=yes,toolbar=yes,menubar=yes' +
            ',scrollbars=yes,resizable=yes,location=yes';
    nsWin = window.open(url,'displayWindow',parms);
    nsWin.focus();
    return false;
}

function openStrippedNS(url) {
    if ((navigator.appName == "Microsoft Internet Explorer") &&
        (parseFloat(navigator.appVersion) < 4 ))
    {
        return false;
    }

    var xOffset=25,yOffset=25;
    var parms = 'left='+ xOffset +
            ',top=' + yOffset +
            ',status=yes,toolbar=no,menubar=no' +
            ',scrollbars=yes,resizable=yes,location=no';
    nsWin = window.open(url,'displayWindow',parms);
    nsWin.focus();
    return false;
}

function openPopup(url, windowName, width, height) {
  if ((navigator.appName == "Microsoft Internet Explorer") &&
    (parseFloat(navigator.appVersion) < 4 )) { return false; }
     
  var xOffset=25,yOffset=25;
  var parms = 'width=' + width +
        ',height=' + height +
        ',left=' + xOffset +
        ',top=' + yOffset +
        ',status=no,toolbar=no,menubar=no' +
        ',scrollbars=yes,resizable=yes';
  var displayName = windowName;

  nsWin = window.open(url, displayName, parms);
  nsWin.focus();
  return false;
}
