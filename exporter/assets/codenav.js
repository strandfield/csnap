
var csnapNavTooltip = {
  htmlElement: null,
  srcElement: null, // the element that trigerred the tooltip
  showTimerId: null,
  hideTimerId: null,
  showDelay: 350,
  normalHideDelay: 200,
  focusHideDelay: 500,
  hideDelay: 200,
  gap: 12,
  
  init: function() {
    this.htmlElement = document.createElement('div');
    this.htmlElement.setAttribute('id', 'tooltip');
    this.htmlElement.setAttribute('style', 'position: absolute');
    var content = document.getElementById('content');
    content.appendChild(this.htmlElement);
    var self = this;
    this.htmlElement.addEventListener('mouseover', function(e) { 
      self.hideDelay = self.focusHideDelay;
      clearTimeout(self.hideTimerId);
    });
    this.htmlElement.addEventListener('mouseout', function(e) { 
      self.hideAfterDelay();
    });
  },
  
  geometry: function(elem) {
    var rect = elem.getBoundingClientRect();

    var geom = { 
      top: rect.top + window.scrollY, 
      left: rect.left + window.scrollX, 
      width: elem.offsetWidth,
      height: elem.offsetHeight,
    };

    return geom;
  },

  setUnderElem: function(elem) {
    var geom = this.geometry(elem);
    var content = document.getElementById('content');
    var docwidth = content.clientWidth-15;
    var contentTop = content.offsetTop;
    var winheight= window.innerHeight - 18 - contentTop;
    var toppos = window.scrollY + contentTop;
    var twidth = this.htmlElement.offsetWidth;
    var theight = this.htmlElement.offsetHeight;
    var tipx = geom.left + geom.width/2 - twidth/2 ;
    tipx += content.scrollLeft;
    if (tipx+twidth>docwidth) tipx = docwidth - twidth - this.gap;
    else if (tipx < 0) tipx = this.gap;
    var tipy=geom.top + geom.height/2 + this.gap;
    tipy += content.scrollTop;
    tipy = (tipy - toppos + theight > winheight && tipy - theight > toppos) ? tipy - theight - (2*this.gap) : tipy //account for bottom edge
    this.srcElement = elem;
    this.htmlElement.style.left = tipx + 'px';
    this.htmlElement.style.top = tipy + 'px';
  },
  
  showAfterDelay: function(elem, additionalFunction) {
    clearTimeout(this.showTimerId)
    var tt = this;
    this.showTimerId = setTimeout( function() {
      clearTimeout(tt.hideTimerId);
      if (additionalFunction)
        additionalFunction();
      tooltip.style.display = 'block';
      tt.setUnderElem(elem);
      tt.hideDelay = tt.normalHideDelay;
    }, this.showDelay);
  },
  
  hideAfterDelay: function(e) {
    clearTimeout(this.showTimerId);
    clearTimeout(this.hideTimerId);
    var tooltip = this.htmlElement;
    this.hideTimerId = setTimeout( function() {
      tooltip.style.display = 'none';
    }, this.hideDelay);
  },
  
  setHtml: function(html) {
    this.htmlElement.innerHTML = html;
  }
};

var csnapCodeNav = {
  currentHighlightSym: '',
  tooltip: csnapNavTooltip,
  coverageVisible: false,

  init: function() {
    csnapNavTooltip.init();
  },

  highlight: function(sym) {
    if(sym == this.currentHighlightSym)
      return;
    
    if(this.currentHighlightSym != '') {
      var highlighted_elements = document.querySelectorAll('[sym-ref="' + this.currentHighlightSym + '"]');
      highlighted_elements.forEach(function(e) {
        e.classList.remove('highlight');
      });
    }
  
    if(sym != '') {
      var elems_to_highlight = document.querySelectorAll('[sym-ref="' + sym + '"]');
      elems_to_highlight.forEach(function(e) {
        e.classList.add('highlight');
      });
    }

    this.currentHighlightSym = sym;
  },

  implGetParentTD: function(elem) {
    if(elem.tagName == 'TD')
      return elem;
    return this.implGetParentTD(elem.parentElement);
  },

  implGetLine: function(elem) {
    return this.implGetParentTD(elem).previousSibling.id.slice(1);
  },

  implShowTooltip: function(elem, symref) {
    var references = document.querySelectorAll('[sym-ref="' + symref + '"]');
    var content = "<b>" + references.length + " reference(s) in this document</b><br/>";
    references.forEach(function (ref) {
      content += "Line " + csnapCodeNav.implGetLine(ref) + "<br/>";
    });
    
    content += `<div style='text-align: right;'><a href='${csnapRootPath}symbols/${symref}.html'>More...</a></div>`;
    
    // $TODO: see what we can do to collect references in other documents 
    // asynchronously (e.g., with XMLHttpRequest).
    
    this.tooltip.showAfterDelay(elem, function() { csnapNavTooltip.setHtml(content); });
  },

  showTooltip: function(elem) {
    var symref = elem.getAttribute('sym-ref');
    
    if(symref != null && symref != '') {
      this.implShowTooltip(elem, symref);
    } else {
      this.tooltip.hideAfterDelay();
    }
  },
  
  toggleCoverage: function() {
    var styleelem = document.getElementById('covstyle');
    
    this.coverageVisible = !this.coverageVisible;
    
    if (this.coverageVisible) {
      styleelem.innerHTML = '.code .cov td { background-color: rgb(200, 255, 200); } .code .uncov td { background-color: rgb(255, 200, 200); }';
    } else {
      styleelem.innerHTML = ' ';
    }
  },
};


window.onload = function() {

  var code = document.querySelector('.code');

  code.addEventListener('mouseover', function(e) { 
    csnapCodeNav.highlight(e.target.getAttribute('sym-ref'));
    csnapCodeNav.showTooltip(e.target);
  });
  
  code.addEventListener('mouseout', function(e) { 
    csnapCodeNav.highlight('');
  });
    
  csnapCodeNav.init();
};
