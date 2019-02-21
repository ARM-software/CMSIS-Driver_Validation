var addNetDs = true;                                    
var netDSFolder = "/Network_Ds/html/";                       // for special handling

var strgURL =   location.pathname;                      // path of current component

// constuctor for the array of objects
function tabElement(id, folderName, tabTxt )  {
    this.tabTxt = tabTxt;                               // Text displayed as menu on the web
	this.folderName = folderName;                       // folder name of the component 
	this.id = id;                                       // elementID as needed in html; 
	this.currentListItem = '<li id="' + this.id + '" class="current"> <a href="../..' + this.folderName + 'index.html"><span>' + this.tabTxt + '</span></a></li>';
	this.listItem = '<li id="' + this.id + '"> <a href="../..' + this.folderName + 'index.html"><span>' + this.tabTxt + '</span></a></li>';
};

// array of objects
var arr = [];

// fill array
 arr.push( new tabElement( "GEN",     "/General/html/",     "General")       );
 arr.push( new tabElement( "FS",      "/FileSystem/html/",  "File System")   );
 arr.push( new tabElement( "GRA",     "/Graphic/html/",     "Graphic")       );
 arr.push( new tabElement( "NET",     "/Network/html/",     "Network")       );
 arr.push( new tabElement( "NETDS",   "/Network_Ds/html/",  "Network DS")    );
 arr.push( new tabElement( "USB",     "/USB/html/",         "USB")           );
 arr.push( new tabElement( "BSP",     "/Board/html/",       "Board Support") );
 
// steers printing of "Network DS" tab
// this function will disapear; no need for improvement; 
function Network_DS(idx)  {
  var netDsTxt    = arr[idx].currentListItem;

  if (addNetDs)  {                                                    // if print tab "Network DS" 
    if (strgURL.search(arr[idx].folderName) < 0) {                    // and the path does not contain the Network_DS folder
      netDsTxt = arr[idx].listItem;                                   // then, do not highlight the tab
    }                                                                 // (by default the tab is highlighted)
	document.write(netDsTxt);                                         // write the "Network DS" tab
  }                                                                   // else, do not print the "Network DS" at all
};

// write tabs
// called from the header file.
function writeComponentTabs()  {
  for ( var i=0; i < arr.length; i++ ) {
    if (arr[i].folderName == netDSFolder)  {                          // handle this special folder "Network_Ds"
      Network_DS(i);      	  
    } else  {
      if (strgURL.search(arr[i].folderName) > 0) {                    // if this is the current folder
        document.write(arr[i].currentListItem);                       // print and hightlight the tab
      } else {                                                      
        document.write(arr[i].listItem);                              // else, print the tab
      }                                                             
    }
  }
};
