//-------1---------2---------3---------4---------5---------6---------7--------//
//- Copyright WEB/codeX, clickIT 2011 - 2026                                 -//
//-------1---------2---------3---------4---------5---------6---------7--------//
//-                                                                          -//
//-------1---------2---------3---------4---------5---------6---------7--------//
//- USER Functions                                                           -//
//-------1---------2---------3---------4---------5---------6---------7--------//


//------------------------------------------------------------------------------
//- CONSTRUCTOR "UserDefaults"
//------------------------------------------------------------------------------

function UserDefaults()
{
}


//------------------------------------------------------------------------------
//- CONSTRUCTOR "UserValidate"
//------------------------------------------------------------------------------

function UserValidate()
{
    this.ValidateFunc = {
        'Dummy': this.Dummy
    };
}


//------------------------------------------------------------------------------
//- METHOD "Dummy"
//------------------------------------------------------------------------------

UserValidate.prototype.Dummy = function(Value, FormObj)
{
    console.debug('::UserValidate DummyFunc() Value:%s FormObj:%o', Value, FormObj);
    return false;
}


//------------------------------------------------------------------------------
//- CONSTRUCTOR "UserValidateGroup"
//------------------------------------------------------------------------------

function UserValidateGroup()
{
    this.ValidateFunc =
    {
        'Dummy': this.Dummy,
        'UserTimeHourMinuteSecond': this.UserTimeHourMinuteSecond
    };
}


//------------------------------------------------------------------------------
//- METHOD "Dummy"
//------------------------------------------------------------------------------

UserValidateGroup.prototype.Dummy = function(Items)
{
    console.debug('::UserValidate DummyFunc() Items:%o', Items);
    return {
        "Error": true,
        "Message": "Dummy"
    };
}


//------------------------------------------------------------------------------
//- METHOD "UserTimeHourMinuteSecond"
//------------------------------------------------------------------------------

UserValidateGroup.prototype.UserTimeHourMinuteSecond = function(Items)
{
    console.debug('::UserValidate UserTimeHourMinuteSecond() Items:%o', Items);

    const ErrorMsgHour = sysFactory.getText('TXT.VALIDATE.ERROR.TIME-HOUR');
    const ErrorMsgMinute = sysFactory.getText('TXT.VALIDATE.ERROR.TIME-MINUTE');
    const ErrorMsgSecond = sysFactory.getText('TXT.VALIDATE.ERROR.TIME-SECOND');

    const HourData = Items[0].getObjectData();
    const MinuteData = Items[1].getObjectData();
    const SecondData = Items[2].getObjectData();

    UserTimeFormatHours = new Array();
    UserTimeFormatMinutesSeconds = new Array();

    let LeftDigit = 0;
    let RightDigit = -1;

    for (let step = 0; step < 23; step++) {
        RightDigit += 1;
        if (RightDigit == 10) {
            RightDigit = -1;
            LeftDigit += 1;
        }
        UserTimeFormatHours.push(''+LeftDigit+RightDigit);
    }

    LeftDigit = 0;
    RightDigit = -1;

    for (let step = 0; step < 59; step++) {
        RightDigit += 1;
        if (RightDigit == 10) {
            RightDigit = -1;
            LeftDigit += 1;
        }
        UserTimeFormatMinutesSeconds.push(''+LeftDigit+RightDigit);
    }

    console.debug('::UserValidate UserTimeHourMinuteSecond() UserTimeFormatHours:%o', UserTimeFormatHours);

    if (UserTimeFormatHours.includes(HourData) == false) {
        return {
            "Error": true,
            "Message": ErrorMsgHour
        };
    }

    if (UserTimeFormatMinutesSeconds.includes(MinuteData) == false) {
        return {
            "Error": true,
            "Message": ErrorMsgMinute
        };
    }

    if (UserTimeFormatMinutesSeconds.includes(SecondData) == false) {
        return {
            "Error": true,
            "Message": ErrorMsgSecond
        };
    }

    return false;
}


//------------------------------------------------------------------------------
//- CONSTRUCTOR "UserContextMenu"
//------------------------------------------------------------------------------

function UserContextMenu()
{
}


//------------------------------------------------------------------------------
//- METHOD "process"
//------------------------------------------------------------------------------

UserContextMenu.prototype.process = function(ContextMenuRef)
{
    console.debug('::UserContextMenu process() ContextMenuRef:%o', ContextMenuRef);
}
