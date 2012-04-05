/* jQuery javascript functions for search results page */
$(document).ready(function() {

	var unloadedImgLookupRule = ".ds-img-wrap img";
	var numImagesToLoad = checkUnloadedImgs(unloadedImgLookupRule);
	/* Fix col heights for images */
	setTimeout("fixHeightForImages(1" + "," + numImagesToLoad + ",'" + unloadedImgLookupRule + "')", 1000);
	
});
