//infoBubble.js
var OPAC = 0;
var InfoBubble = {

  bubbleTimeout: 1,
  currentBubble: null,
  timeoutBubbleID: null,
  currentLink: null,

  show: function(aID, bID) {
    var disableFade = 0;
    if(this.currentBubble) {
      disableFade = 1;
    }
    var aBubble = document.getElementById(aID);
    if(aBubble && aBubble != this.currentBubble) {
      this.hide();
      var aBubbleLink = document.getElementById(bID);
      aBubbleLink.style.background = '#EFEFEF';
      this.currentLink = aBubbleLink;
      if(disableFade == 0) {
        aBubble.style.filter = 'alpha(opacity=0)';
        aBubble.style.opacity = 0;
      }
      else {
        aBubble.style.filter = '';
        aBubble.style.opacity = '';
      }
      aBubble.style.display = 'inline';
      var heightFromTop = findPosY(aBubble) - document.body.scrollTop;
      if(getWindowHeight() < (aBubble.offsetHeight + heightFromTop)) {
        aBubble.style.top = (getWindowHeight() - (aBubble.offsetHeight + heightFromTop)) - 5;
      }
      else {
        if(heightFromTop < 0) {
          aBubble.style.top = (Math.abs(heightFromTop)) + 5;
        }
        else {
          aBubble.style.top = 5;
        }
      }
      this.currentBubble = aBubble;
      if(disableFade == 0) {
        setTimeout('InfoBubble.fadeIn()',300);
        OPAC = 0;
      }
    }

    if(this.timeoutBubbleID) {
      clearTimeout(this.timeoutBubbleID);
    }
  },

  hide: function() {
    if(this.currentBubble) {
      this.currentLink.style.background = '#F6F6F6';
      this.currentBubble.style.display = 'none';
      this.currentBubble.style.top = 0;
      this.currentBubble = null;
      if(this.timeoutBubbleID) {
        clearTimeout(this.timeoutBubbleID);
      }
    }
  },

  timeout: function() {
    if(OPAC == 0) {
      InfoBubble.hide();
    }
    else {
      this.timeoutBubbleID = setTimeout('InfoBubble.hide()', this.bubbleTimeout * 1000);
    }
  },

  fadeIn: function() {
    if(this.currentBubble) {
      OPAC = OPAC + .10;
      OPAC = (OPAC>1)?1:OPAC;
      this.currentBubble.style.filter = 'alpha(opacity='+parseInt(100*OPAC)+')';
      this.currentBubble.style.opacity = OPAC;
      if(OPAC<1) {
        setTimeout('InfoBubble.fadeIn()',25);
      }
    }
  }
}

function findPosY(obj) {
  var curtop = 0;
  if(obj.offsetParent) {
    while(1) {
      curtop += obj.offsetTop;
      if(!obj.offsetParent) { break; }
        obj = obj.offsetParent;
      }
  }
  else if(obj.y) {
    curtop += obj.y;
  }
  return curtop;
}

function getWindowHeight() {
  var windowHeight=0;
  if(typeof(window.innerHeight) == 'number') {
    windowHeight = window.innerHeight;
  }
  else {
    if(document.documentElement && document.documentElement.clientHeight) {
      windowHeight = document.documentElement.clientHeight;
    }
    else {
      if(document.body && document.body.clientHeight) {
        windowHeight = document.body.clientHeight;
      }
    }
  }
  return windowHeight;
}

function toggleMLKT(uoi, imgDefault, imgFlip, urlPrefix) {
  var mlktItems = document.getElementById('mlkt');
  if(mlktItems.style.display == "none") {
    var mlktList = document.getElementById('mlktList');
    mlktItems.style.display = '';
    document.getElementById('mlktArrow').src = imgDefault;
    if(mlktList.innerHTML.length == 0) {
      mlktList.innerHTML = '<b>LOADING...</b>';

      new Ajax.Updater('mlktList', urlPrefix+'/mlkt/2/'+uoi, {method:'get'});
    }
    else {
      new Ajax.Request(urlPrefix+'/mlkt/1', {method:'get'});
    }
  }
  else {
    mlktItems.style.display = 'none';
    document.getElementById('mlktArrow').src = imgFlip;
    new Ajax.Request(urlPrefix+'/mlkt/0', {method:'get'});
  }
}

function mlktHover(hoverOver, imgPrefix) {
  var mlktItems = document.getElementById('mlkt');
  var mlktImg = document.getElementById('mlktArrow');
  if(mlktItems.style.display == "none") {
    if(hoverOver == 1) {
      mlktImg.src = imgPrefix + 'btn_arrow_down_on.gif';
    }
    else {
      mlktImg.src = imgPrefix + 'btn_arrow_down_off.gif';
    }
  }
  else {
    if(hoverOver == 1) {
      mlktImg.src = imgPrefix + 'btn_arrow_up_on.gif';
    }
    else {
      mlktImg.src = imgPrefix + 'btn_arrow_up_off.gif';
    }
  }
}

// referenceBubble.js
var RefPreview = {

  previewTimeout: 1,
  currentPreview: null,
  timeoutPreviewID: null,

  showRef: function(event, elemId, boxId) {
  var disableFade = 0;
  if(this.currentPreview) {
    disableFade = 1;
  }
  var refPrev = document.getElementById(boxId);
  var reference = document.getElementById(elemId);
  if(reference == null || reference.innerHTML.length <= 0) { return; }

    if (document.getElementById('refCon') != null) {
      var d = document.getElementById('refCon');
      d.parentNode.removeChild( d );
    }

    var newSpan = document.createElement("span");
    newSpan.setAttribute('id', 'refCon');
    newSpan.innerHTML = reference.innerHTML;
    refPrev.appendChild(newSpan);

    if(refPrev != this.currentPreview) {
      this.hide();
      if(disableFade == 0) {
        refPrev.style.filter = 'alpha(opacity=0)';
        refPrev.style.opacity = 0;
      }
      else {
        refPrev.style.filter = '';
        refPrev.style.opacity = '';
      }
      var x = 0;
      var y = 0;

      var maxX;
      var maxY;

      refPrev.style.position="absolute";
      refPrev.style.display = "block";
      var divX = refPrev.offsetWidth;
      var divY = refPrev.offsetHeight;

      if (document.all && !window.opera) {
        if (document.documentElement && document.documentElement.scrollTop) {
          maxX = document.documentElement.clientWidth + document.documentElement.scrollLeft - divX;
          maxY = document.documentElement.clientHeight + document.documentElement.scrollTop - divY;
          y = event.clientY + document.documentElement.scrollTop + 10;
          x = event.clientX + document.documentElement.scrollLeft + 15;
        } else {
          maxX = document.body.clientWidth + document.body.scrollLeft - divX;
          maxY = document.body.clientHeight + document.body.scrollTop - divY;

          y = event.clientY + document.body.scrollTop + 10;
          x = event.clientX + document.body.scrollLeft + 15;
        }
      } else {
        maxX = window.innerWidth + window.pageXOffset - divX;
        maxY = window.innerHeight + window.pageYOffset - divY;

        y = event.pageY + 10;
        x = event.pageX + 15;
      }

      if (x > maxX) {
        x = maxX - 25;
      }
      if (y > maxY) {
        y = maxY - 30;
      }

      refPrev.style.top = y + "px";
      refPrev.style.left = x + "px";

      this.currentPreview = refPrev;
      if(disableFade == 0) {
        setTimeout('RefPreview.fadeIn()',100);
        OPAC = 0;
      }
    }

    if(this.timeoutPreviewID) {
      clearTimeout(this.timeoutPreviewID);
    }
  },

  hide: function() {
    if(this.currentPreview) {
      this.currentPreview.style.display = 'none';
      this.currentPreview = null;
      if(this.timeoutPreviewID) {
        clearTimeout(this.timeoutPreviewID);
      }
    }
  },

  hideRef: function() {
    if(OPAC == 0) {
      RefPreview.hide();
    }
    else {
      this.timeoutPreviewID = setTimeout('RefPreview.hide()', this.previewTimeout * 2000);
    }
  },

  fadeIn: function() {
    if(this.currentPreview) {
      OPAC = OPAC + .10;
      OPAC = (OPAC>1)?1:OPAC;
      this.currentPreview.style.filter = 'alpha(opacity='+parseInt(100*OPAC)+')';
      this.currentPreview.style.opacity = OPAC;
      if(OPAC<1) {
        setTimeout('RefPreview.fadeIn()',25);
      }
    }
  }
}

// resizableImages.js
//_____________  The following arrays are synched with each other...
var resizableImages = new Array();
var resizableImageBaseWidth = new Array();
var resizableImageLinks = new Array();
//_____________ End of synched arrays

var resizableRefsInitilized = 0;
var NaturalWidthSupported = 0;


// RESIZE_CLIENT_ADJUST_PX
//   -- reflects the amount of space for the FAT TAB Inner div
//      and misc margins (...).
var RESIZE_CLIENT_ADJUST_PX = 395;

function setNaturalWidthSupported(inResizableImage) {
  if (inResizableImage.naturalWidth)
  {
    NaturalWidthSupported = 1;
  } else {
    NaturalWidthSupported = 0;
  }
}

function getBaseWidth(inImage) {
  var baseWidth = 0;
  if (inImage.naturalWidth)
  {
    baseWidth = inImage.naturalWidth;
  } else {
    var imageTargetWidth;
    var imgClone = inImage.cloneNode(0);
    inImage.className = '';
    inImage.style.width = 'auto';
    inImage.style.height = 'auto';
    inImage.style.borderWidth = '0';
    inImage.style.padding = '0';
    inImage.removeAttribute('width');
    inImage.removeAttribute('height');
    baseWidth = parseInt(inImage.width);
    if ((parseInt(document.body.clientWidth)- RESIZE_CLIENT_ADJUST_PX)
        <  baseWidth)
    {
      imageTargetWidth = parseInt(document.body.clientWidth)-
                         RESIZE_CLIENT_ADJUST_PX;
    }
    else
    {
      imageTargetWidth = baseWidth;
    }
    inImage.setAttribute('width' , '' + imageTargetWidth + 'px' );
    inImage.setAttribute('height', 'auto' );
    inImage.setAttribute('height', imgClone.getAttribute('height') );
    inImage.style.width = "" + imageTargetWidth + "px";
    inImage.style.height = 'auto';
    inImage.className = imgClone.className ;
    inImage.style.padding = imgClone.style.padding ;
    inImage.style.borderWidth=  imgClone.style.borderWidth ;
  }
  return(baseWidth);
}

function initResizableRefs() {
  if (! resizableRefsInitilized)
  {
    var resizableContainers;
    var currentResizableImages; // should be collection of 1
    var currentResizableLinks; // should be collection of 1

    resizableContainers = getElementsByClassName(document,
                                                 'DIV',
                                                 'sizeImgBox');
    for(var k=0;k<resizableContainers.length;k++)
    {
      currentResizableImages = getElementsByClassName(resizableContainers[k],
                                                      'IMG',
                                                      'sizeImg');
      currentResizableLinks  = getElementsByClassName(resizableContainers[k],
                                                      'DIV',
                                                      'sizeImgLink');
      if ((currentResizableImages.length > 0) &&
          (currentResizableLinks.length > 0))
      {
        resizableImages.push(currentResizableImages[0]);
        resizableImageLinks.push(currentResizableLinks[0]);
        resizableImageBaseWidth.push(
                       getBaseWidth(currentResizableImages[0],
                                    resizableContainers[k]));
        if (resizableImages.length == 1)
        {
          setNaturalWidthSupported(currentResizableImages[0]);
        }
      }
    }
  }
  resizableRefsInitilized = 1;
}

function setVisibilityOfResizableImageLinks() {
  var currentResizableImage;
  var currentResizableLink;

  initResizableRefs();

  for(var k=0;k<resizableImageLinks.length;k++)
  {
    if ((parseInt(document.body.clientWidth)-RESIZE_CLIENT_ADJUST_PX)
        < resizableImageBaseWidth[k])
    {
      resizableImageLinks[k].style.display = "inline"; // for scaled img
    }
    else
    {
      resizableImageLinks[k].style.display = "none";   // for full img
    }
  }
}

function resizeImages() {
  initResizableRefs();
  if ( ! NaturalWidthSupported)
  {
    var currentResizableImage;
    var currentResizableLink;

    for(var k=0;k<resizableImages.length;k++)
    {
      if ((parseInt(document.body.clientWidth)-RESIZE_CLIENT_ADJUST_PX)
          < resizableImageBaseWidth[k])
      {
        targetWidth = parseInt(document.body.clientWidth)
                      - RESIZE_CLIENT_ADJUST_PX;
      }
      else
      {
        targetWidth = resizableImageBaseWidth[k];
      }
      resizableImages[k].style.width = "" + targetWidth + "px";
    }
  }
}

function resizeImgs() {
  setVisibilityOfResizableImageLinks();
  resizeImages();
}

// scrollableTables.js
//_____________  The following arrays are synched with each other...
var scrollableTableDivs = new Array();
var scrollableTables = new Array();
var scrollableTableLinks = new Array();
//_____________ End of synched arrays

var scrollableTableRefsInitilized = 0;

// EXTERNAL: RESIZE_CLIENT_ADJUST_PX  -- Defined in resizableTable
//   -- reflects the amount of space for the FAT TAB Inner div
//      and misc margins (...).
// var RESIZE_CLIENT_ADJUST_PX = 395;

function initScrollableTableRefs() {
  if (! scrollableTableRefsInitilized) {
    var scrollableTableContainers;
    var currentScrollableTableDivs; // should be collection of 1
    var currentScrollableTables; // Want first (outermost table)
    var currentScrollableTableLinks; // should be collection of 1

    scrollableTableContainers = getElementsByClassName(document,
                                                     'DIV',
                                                     'tblScrollBox');
    for(var k=0;k<scrollableTableContainers.length;k++)
    {
      currentScrollableTableDivs = getElementsByClassName(
                                                  scrollableTableContainers[k],
                                                  'DIV',
                                                  'tblScroll');
      currentScrollableTableLinks = getElementsByClassName(
                                                  scrollableTableContainers[k],
                                                  'DIV',
                                                  'tblScrollLink');
      if ((currentScrollableTableDivs.length > 0) &&
          (currentScrollableTableLinks.length > 0))
      {
        currentScrollableTables = getElementsByClassName(
                                                  currentScrollableTableDivs[0],
                                                  'TABLE',
                                                  '*');
        if (currentScrollableTables.length > 0) {
          scrollableTableDivs.push(currentScrollableTableDivs[0]);
          scrollableTables.push(currentScrollableTables[0]);
          scrollableTableLinks.push(currentScrollableTableLinks[0]);
        }
      }
    }
  }
  scrollableTableRefsInitilized = 1;
}

function setVisibilityOfScrollableTableLinks() {
  initScrollableTableRefs();

  for(var k=0;k<scrollableTableLinks.length;k++) {
    if (scrollableTableDivs[k].offsetWidth < scrollableTables[k].offsetWidth) {
      scrollableTableLinks[k].style.display = "inline"; // Scrolled Table
    } else {
      scrollableTableLinks[k].style.display = "none";   // Full Table
    }
  }
}

function resetScrollableTables() {
  setVisibilityOfScrollableTableLinks();
}

// tabChanges.js
function getElementsByClassName(oElm, strTagName, strClassName){
  var arrElements = (strTagName == "*" && oElm.all)? oElm.all : oElm.getElementsByTagName(strTagName);
  var arrReturnElements = new Array();
  strClassName = strClassName.replace(/\-/g, "\\-");
  var oRegExp = new RegExp("(^|\\s)" + strClassName + "(\\s|$)");
  var oElement;
  for(var i=0; i<arrElements.length; i++){
   oElement = arrElements[i];
   if(oRegExp.test(oElement.className)){
    arrReturnElements.push(oElement);
   }
  }
  return (arrReturnElements);
}

function hideElementsByClassName(className, tag)  {
  var all = getElementsByClassName(document, tag, className);
  for(var k=0;k<all.length;k++) {
    all[k].style.display = "none";
  }
}

function showElementsByClassName(className, tag)  {
  var all = getElementsByClassName(document, tag, className);
  for(var k=0;k<all.length;k++) {
    all[k].style.display = "inline";
  }
}

var graphTabOffLink_OnLoad = "";
var refTabOffLink_OnLoad = "";
function enableTabs()
{
  var all;
  var k;

  var graphTabOffLink = document.getElementById('graphTabOffLinkNode');
  var refTabOffLink = document.getElementById('refTabOffLinkNode');

  if (graphTabOffLink)
  {
    if (graphTabOffLink_OnLoad != "")
    {
      graphTabOffLink.href = graphTabOffLink_OnLoad;
      all = getElementsByClassName(graphTabOffLink, 'SPAN', 'disabledTabLink');
      for(k=0;k<all.length;k++)
      {
        all[k].className = ""; // remove disabledTabLink class
      }
    }
  }

  if (refTabOffLink)
  {
    if (refTabOffLink_OnLoad != "")
    {
      refTabOffLink.href = refTabOffLink_OnLoad;
      all = getElementsByClassName(refTabOffLink, 'SPAN', 'disabledTabLink');
      for(k=0;k<all.length;k++)
      {
        all[k].className = ""; // remove disabledTabLink class
      }
    }
  }
}

function loadUnsubArticle() {
changeView('unsubTab');
resizeImgs();
resetScrollableTables();
enableTabs();
}

function changeView(tab) {
  if(tab == 'graphTab') {
    showElementsByClassName('graphText', 'div');
    showElementsByClassName('graphTextOnly', 'div');
    hideElementsByClassName('refText', 'div');
    hideElementsByClassName('articleText', 'div');
    hideElementsByClassName('refMsg', 'div');
  }
  else if(tab == 'refTab') {
    showElementsByClassName('refText', 'div');
    hideElementsByClassName('articleText', 'div');
    hideElementsByClassName('graphText', 'div');
    hideElementsByClassName('graphTextOnly', 'div');
    showElementsByClassName('refMsg', 'div');
  }
  else {
    showElementsByClassName('articleText', 'div');
    showElementsByClassName('graphText', 'div');
    hideElementsByClassName('graphTextOnly', 'div');
    showElementsByClassName('refText', 'div');
    hideElementsByClassName('refMsg', 'div');
  }
}

function flip(e){
  var menustyle = eval(getStyleObj(e))
  if (menustyle.display=="none"){
    menustyle.display=''
  }
  else{
    menustyle.display="none"
  }
}
function getStyleObj(elem,parent) {
  if (document.layers) {
    if (parent) {
      return "document."+parent+".document."+elem;
    } else {
      return "document."+elem + ".style";
    }
  } else if (document.all) {
    return "document.all."+elem + ".style";
  } else if (document.getElementById) {
    return "document.getElementById('"+elem+"').style";
  }
}

function toggleTabs(tabSelected, keyEvent) {

  var absTabOn = document.getElementById('absTabOn');
  var absTabOff = document.getElementById('absTabOff');
  var fullTabOn = document.getElementById('fullTabOn');
  var fullTabOff = document.getElementById('fullTabOff');
  var graphTabOn = document.getElementById('graphTabOn');
  var graphTabOff = document.getElementById('graphTabOff');
  var refTabOn = document.getElementById('refTabOn');
  var refTabOff = document.getElementById('refTabOff');

  if (tabSelected == 'unsubTab') {
    if (absTabOff) { absTabOff.style.display = 'none'; }
    if (absTabOn) { absTabOn.style.display = ''; }
    if(graphTabOn) { graphTabOn.style.display = 'none'; }
    if(graphTabOff) { graphTabOff.style.display = ''; }
    if(refTabOn) { refTabOn.style.display = 'none'; }
    if(refTabOff) { refTabOff.style.display = ''; }
  }
  if (tabSelected == 'fullTab') {
    if (fullTabOn) { fullTabOn.style.display = ''; }
    if (fullTabOff) {fullTabOff.style.display = 'none'; }
    if(graphTabOn) { graphTabOn.style.display = 'none'; }
    if(graphTabOff) { graphTabOff.style.display = ''; }
    if(refTabOff) { refTabOff.style.display = ''; }
    if(refTabOn) { refTabOn.style.display = 'none'; }
  }
  if (tabSelected == 'graphTab') {
    if(absTabOn) { absTabOn.style.display = 'none'; }
    if(absTabOff) { absTabOff.style.display = ''; }
    if(fullTabOn) { fullTabOn.style.display = 'none'; }
    if(fullTabOff) { fullTabOff.style.display = ''; }
    if (graphTabOff) { graphTabOff.style.display = 'none'; }
    if (graphTabOn) { graphTabOn.style.display = ''; }
    if(refTabOn) { refTabOn.style.display = 'none'; }
    if(refTabOff) { refTabOff.style.display = ''; }

    createEvent (tabSelected, keyEvent);
  }
  if (tabSelected == 'refTab') {
    if(absTabOn) { absTabOn.style.display = 'none'; }
    if(absTabOff) { absTabOff.style.display = ''; }
    if(fullTabOn) { fullTabOn.style.display = 'none'; }
    if(fullTabOff) { fullTabOff.style.display = ''; }
    if(graphTabOn) { graphTabOn.style.display = 'none'; }
    if(graphTabOff) { graphTabOff.style.display = ''; }
    if (refTabOff) { refTabOff.style.display = 'none'; }
    if (refTabOn) {refTabOn.style.display = ''; }

    createEvent (tabSelected, keyEvent);
  }

  setFigSelection (tabSelected);

  changeView(tabSelected);
}

function createEvent (tabSelected, keyEvent) {
  if (tabSelected == 'graphTab') {
    if (document.getElementById('figevent') == null) {
      new Ajax.Request(keyEvent);
      var figTabKey = document.createElement('div')
      figTabKey.setAttribute('id', 'figevent');
      document.getElementById('graphTabOn').appendChild(figTabKey)
    }
  }
  if (tabSelected == 'refTab') {
    if (document.getElementById('refevent') == null) {
      new Ajax.Request(keyEvent);
      var refTabKey = document.createElement('div')
      refTabKey.setAttribute('id', 'refevent');
      document.getElementById('refTabOn').appendChild(refTabKey)
    }
  }
}

function setFigSelection (tabSelected) {
  if (tabSelected == 'graphTab') {
    var graphTabKey = document.createElement('div')
    graphTabKey.setAttribute('id', 'figSelect');
    document.getElementById('graphTabOn').appendChild(graphTabKey)

    var imgThumb = document.getElementById('imgToggle');
    var figThumb = document.getElementById('figToggle');
    if (imgThumb != null || figThumb != null) {
      // Hide article thumbnail, full size image links
      imgThumb.style.display = 'none';
      // Show the figure thumbnail, full size image links
      figThumb.style.display = '';
    }
  }
  else {
    if (document.getElementById('figSelect') != null) {
      var d = document.getElementById('figSelect');
      d.parentNode.removeChild( d );

      var imgThumb = document.getElementById('imgToggle');
      var figThumb = document.getElementById('figToggle');
      if (imgThumb != null || figThumb != null) {
        // Hide article thumbnail, full size image links
        figThumb.style.display = 'none';
        // Show the figure thumbnail, full size image links
        imgThumb.style.display = '';
      }
    }
  }
}
