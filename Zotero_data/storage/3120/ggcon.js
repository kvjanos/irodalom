function google_ad_request_done(google_ads) {

    var s = '';
    var i;

    if (google_ads.length == 0) {
      return;
    }

    if (google_ads[0].type == "image") {
      s += '<a href="' + google_ads[0].url +
              '" target="_top" title="go to ' + google_ads[0].visible_url +
              '"><img border="0" src="' + google_ads[0].image_url +
              '"width="' + google_ads[0].image_width +
              '"height="' + google_ads[0].image_height + '"></a>';
    } else if (google_ads[0].type == "flash") {
      s += '<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"' +
              ' codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0"' +
              ' WIDTH="' + google_ad.image_width +
              '" HEIGHT="' + google_ad.image_height + '">' +
              '<PARAM NAME="movie" VALUE="' + google_ad.image_url + '">' +
              '<PARAM NAME="quality" VALUE="high">' +
              '<PARAM NAME="AllowScriptAccess" VALUE="never">' +
              '<EMBED src="' + google_ad.image_url +
              '" WIDTH="' + google_ad.image_width +
              '" HEIGHT="' + google_ad.image_height + 
              '" TYPE="application/x-shockwave-flash"' + 
              ' AllowScriptAccess="never" ' + 
              ' PLUGINSPAGE="http://www.macromedia.com/go/getflashplayer"></EMBED></OBJECT>';
    } else if (google_ads[0].type == "text") {
        s += '<div style="background-color:#FFFFFF; padding-top: 15px; padding-bottom: 15px; padding-left: 5px; padding-right: 5px">'
        s += '<div style="width: 100%;border: 1px solid #CCCCCC; font-family: arial; font-size: 12px;">';
        s += '<div style="padding: 4px; border-bottom: 1px solid #CCCCCC; background-color:#EEEEEE">Sponsored Links</div>';
        if (google_ads.length == 1) {
            /*
             * Partners should adjust text sizes
             * so ads occupy the majority of ad space.
             */
            s += '<div style="margin: 5px">';
            s += '<a href="' + google_ads[0].url + '" ' +
                            'onmouseout="window.status=\'\'" ' +
                            'onmouseover="window.status=\'go to ' +
                            google_ads[0].visible_url + '\'" ' +
                            '>' +
                            google_ads[0].line1 + '</a><br>' +
                            '<span style="color:#000000">' +
                            google_ads[0].line2 + '&nbsp;' +
                            google_ads[0].line3 + '<br></span>' +
                            '<span style="color:#008000">' +
                            google_ads[0].visible_url + '</span><br>';
            s += '</div>';
        } else if (google_ads.length > 1) {
            /*
             * For text ads, append each ad to the string.
             */
            for(i=0; i < google_ads.length; ++i) {
                s += '<div style="margin: 5px">';
                s += '<a href="' + google_ads[i].url + '" ' +
                                  'onmouseout="window.status=\'\'" ' +
                                  'onmouseover="window.status=\'go to ' +
                                  google_ads[i].visible_url + '\'" ' +
                                  '>' +
                                  google_ads[i].line1 + '</a><br>' +
                                  '<span style="color:#000000">' +
                                  google_ads[i].line2 + '<br>' +
                                  google_ads[i].line3 + '<br></span>' +
                                  '<span style="color:#008000">' +
                                  google_ads[i].visible_url + '</span><br>';
                s += '</div>';
            }
        }
        s += '</div></div>';
    }

    document.write(s);
    return;
}
