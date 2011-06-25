QNumberBox : QAbstractStepValue {
  var <clipLo, <clipHi, <scroll, <scroll_step;
  var <align, <buttonsVisible = false;
  var <normalColor, <typingColor;

  *qtClass { ^"QcNumberBox" }

  *new { arg aParent, aBounds;
    var obj = super.new( aParent, aBounds );
    obj.initQNumberBox;
    ^obj;
  }

  initQNumberBox {
    clipLo = inf;
    clipHi = inf;
    scroll = true;
    scroll_step = 1;
    normalColor = Color.black;
    typingColor = Color.red;
  }

  value {
    var type = this.getProperty( \valueType );
    var val;
    switch( type,
      0 /* Number */, { val = this.getProperty( \value ) },
      1 /* Inf */, { val = inf },
      2 /* -Inf */, { val = -inf },
      3 /* NaN */, { val = 0 },
      4 /* Text */, { val = 0 }
    );
    ^val;
  }

  value_ { arg value;
    case
      // isNaN has to be on the first plase, because a NaN is also equal to inf and -inf
      { value.isNaN } { this.invokeMethod( \setNaN ); }
      { value == inf } { this.invokeMethod( \setInfinite, true ); }
      { value == -inf } { this.invokeMethod( \setInfinite, false ); }
      { this.setProperty( \value, value.asFloat ); }
    ;
  }

  string { ^this.getProperty( \text ); }

  string_ { arg string; this.setProperty( \text, string ); }

  clipLo_ { arg aFloat;
    clipLo = aFloat;
    this.setProperty( \minimum, aFloat; );
  }

  clipHi_ { arg aFloat;
    clipHi = aFloat;
    this.setProperty( \maximum, aFloat; );
  }

  scroll_ { arg aBool;
    scroll = aBool;
    this.setProperty( \scroll, aBool );
  }

  scroll_step_ { arg aFloat;
    scroll_step = aFloat;
    this.setProperty( \scrollStep, aFloat );
  }

  decimals { ^this.getProperty(\decimals); }
  minDecimals { ^this.getProperty(\minDecimals); }
  maxDecimals { ^this.getProperty(\maxDecimals); }

  decimals_ {  arg decimals; this.setProperty( \decimals, decimals ); }
  minDecimals_ { arg decimals; this.setProperty( \minDecimals, decimals ); }
  maxDecimals_ { arg decimals; this.setProperty( \maxDecimals, decimals ); }

  align_ { arg alignment;
    align = alignment;
    this.setProperty( \alignment, QAlignment(alignment));
  }

  stringColor {
    ^this.palette.baseTextColor;
  }

  stringColor_ { arg color;
    this.setProperty( \palette, this.palette.baseTextColor_(color) );
  }

  normalColor_ { arg aColor;
    normalColor = aColor;
    this.setProperty( \normalColor, aColor );
  }

  typingColor_ { arg aColor;
    typingColor = aColor;
    this.setProperty( \editingColor, aColor );
  }

  background {
    ^this.palette.baseColor;
  }

  background_ { arg color;
    this.setProperty( \palette, this.palette.baseColor_(color) )
  }

  buttonsVisible_ { arg aBool;
    buttonsVisible = aBool;
    this.setProperty( \buttonsVisible, aBool );
  }

  defaultGetDrag { ^this.value; }
  defaultCanReceiveDrag { ^QView.currentDrag.isNumber; }
  defaultReceiveDrag {
    this.valueAction = QView.currentDrag;
  }
}