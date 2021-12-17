const sendToPlugin = AVT_CREATOR_CENTRAL_API.sendToPlugin;
const propertyEvents = new EventEmitter();
const setSetting = AVT_CREATOR_CENTRAL_API.setSetting;
const getSetting = AVT_CREATOR_CENTRAL_API.getSetting;
let i18n = {};

AVT_CREATOR_CENTRAL.on('ax.send.to.property', data => {
    let action = '';
    if(data.params.payload.hasOwnProperty('action')) {
        action = data.params.payload.action;
        propertyEvents.emit(action, data.params.payload);
    }
});

function get_city_val(sel){
    sendToPlugin({
        'action' : 'set_city_val',
        'city': sel.value
    });  
}

function get_type_val(val){
    sendToPlugin({
        'action' : 'set_type_val',
        'type': val.value
    });  
}


propertyEvents.on('send_city_val', read_city_val)
function read_city_val(data){
    $('#js_collection').val(data.city);
}

propertyEvents.on('send_type_val', read_type_val)
function read_type_val(data){
    $('#js_type').val(data.type);
}


propertyEvents.on('setLanguage', data => {
    i18n = localization[data.data];
    for(let k in i18n) {
        $('.' + k).text(i18n[k]);
    }
});
