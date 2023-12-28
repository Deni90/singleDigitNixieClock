class Backlight {
    constructor(state, r, g, b, a) {
      this.state = state;
      this.r = r;
      this.g = g;
      this.b = b;
      this.a = a;
    }
  }


let backlight = new Backlight(0,0,0,0,0);

$(document).on('pagebeforecreate', '#index', function(){
    var interval = setInterval(function(){
        $.mobile.loading('show');
        clearInterval(interval);
    },1);
});

$(document).on('pageshow', '#index', function(){
    var interval = setInterval(function(){
        $.mobile.loading('hide');
        clearInterval(interval);
    },1);
});

$(document).ready(function() {
    console.log("page ready");

    $.ajax({url: "/backlight", type: "GET", dataType: "json"})
    .success(function(result){
        console.log(result);
        backlight.state = result.state;
        backlight.r = result.R;
        backlight.g = result.G;
        backlight.b = result.B;
        backlight.a = result.A;
        $(function() {
            $('input:radio[name="backlightType"]').filter('[value="'+ backlight.state + '"]').attr("checked",true).checkboxradio("refresh");
        });
    });

    $("input[name= 'backlightType']").on('change', function(){
        backlight.state = $(this).val();
        console.log("Selected backlight state = " + backlight.state);
        var jsonObj = {"state": backlight.state}
        $.ajax({url: "/backlight/state", type: "POST", dataType: "json", data: jsonObj})
            .success(function(result){
                console.log(result);
        });
    });
});