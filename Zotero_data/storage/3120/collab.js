var collabTimer;
var collabKeyEventURL;
var collabJsStr = "";
var collabCssStr = "";
var collabHtmlStr = "";

function copyCollab() {
  return(collabJsStr + collabHtmlStr);
}
function hideDiv(id) {
  document.getElementById(id).style.display = 'none';
}
function stop2CollabTimeout() {
  clearTimeout(collabTimer);
  document.getElementById("CollabBoxContent").innerHTML = this.copyCollab();
}
function collabKE(targetUrl) {
  var url;
  url = collabKeyEventURL;
  url += targetUrl;

  new Ajax.Request(url, {method:'get'});
}
