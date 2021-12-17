window.AVT_CREATOR_CENTRAL;
window.REQUEST_SEQ_ID = 0;
window.PLUGIN_UUID;
window.Utils = {};

Utils.parseJson = function(jsonString) {
    if (typeof jsonString === 'object') return jsonString;
    try {
        const o = JSON.parse(jsonString);

        // Handle non-exception-throwing cases:
        // Neither JSON.parse(false) or JSON.parse(1234) throw errors, hence the type-checking,
        // but... JSON.parse(null) returns null, and typeof null === "object",
        // so we must check for that, too. Thankfully, null is falsey, so this suffices:
        if (o && typeof o === 'object') {
            return o;
        }
    } catch (e) {}

    return false;
};

function connectCreatorCentral(port, UID, inRegisterEvent, inInfo, inActionInfo) {
    PLUGIN_UUID = UID;
    AVT_CREATOR_CENTRAL.connect(port, UID, inRegisterEvent, inInfo, inActionInfo);
}

WebSocket.prototype.sendJSON = function(jsn, log) {
    if (log) {
        console.log('SendJSON', this, jsn);
    }
    this.send(JSON.stringify(jsn));
};

class EventEmitter {
    constructor() {
        this.events = {};
    }
    on(event, listener) {
        if (typeof this.events[event] !== 'object') {
            this.events[event] = [];
        }
        this.events[event].push(listener);
        return () => this.removeListener(event, listener);
    }
    removeListener(event, listener) {
        if (typeof this.events[event] === 'object') {
            const idx = this.events[event].indexOf(listener);
            if (idx > -1) {
            this.events[event].splice(idx, 1);
            }
        }
    }
    emit(event, ...args) {
        if (typeof this.events[event] === 'object') {
        this.events[event].forEach(listener => listener.apply(this, args));
        }
    }
    once(event, listener) {
        const remove = this.on(event, (...args) => {
            remove();
            listener.apply(this, args);
        });
    }
};

const AVT_CREATOR_CENTRAL_API = {
    queue : {},
    send: function (apiType, payload) {
        const pl = Object.assign({}, {
            id : REQUEST_SEQ_ID,
            method : apiType,
            jsonrpc : "2.0",
            params : {
                id : PLUGIN_UUID,
                payload : payload
            }
        });

        AVT_CREATOR_CENTRAL.connection && AVT_CREATOR_CENTRAL.connection.sendJSON(pl);

    },
  	setSetting : function(payload) {
            AVT_CREATOR_CENTRAL_API.send('ax.set.payload', payload);
  	},
  	getSetting: function() {
            AVT_CREATOR_CENTRAL_API.send('ax.get.payload', {});
    },
    setImage: function (image) {
        AVT_CREATOR_CENTRAL_API.send('ax.set.image', {
            'image': image
        });
    },
    sendToProperty: (payload) => {
        AVT_CREATOR_CENTRAL_API.send('ax.send.to.property', payload);
    },
    sendToPlugin : function (payload) {
        AVT_CREATOR_CENTRAL_API.send('ax.send.to.widget', payload);
    },
    setState: function (state) {
        AVT_CREATOR_CENTRAL_API.send('ax.change.state', {
            'state': Number(state)
        });
    },
};

//main
AVT_CREATOR_CENTRAL = (function() {

    function init() {
        let inPort,
            inUUID,
            inMessageType,
            websocket = null;

        let events = new EventEmitter();

        function connect(port, UID, inRegisterEvent, inInfo, inActionInfo) {
            inPort = port;
            inUUID = UID;
            inMessageType = "ax.register.property";
            websocket = new WebSocket('ws://127.0.0.1:' + inPort);

            websocket.onopen = function() {
                var json = {
                    jsonrpc : '2.0',
                    id: REQUEST_SEQ_ID,
                    method : inMessageType,
                    params : {
                        id : inUUID
                    }
                };
                AVT_CREATOR_CENTRAL_API.queue[REQUEST_SEQ_ID] = inMessageType + '.result';
                websocket.sendJSON(json);
                AVT_CREATOR_CENTRAL.uuid = inUUID;
                AVT_CREATOR_CENTRAL.connection = websocket;
            };

            websocket.onerror = function(evt) {
                console.warn('WEBOCKET ERROR', evt, evt.data);
            };

            websocket.onclose = function(evt) {
                // Websocket is closed
                console.warn('error', evt);
            };

            websocket.onmessage = function(evt) {
                let jsonObj = Utils.parseJson(evt.data);
                let method = '';
                if(jsonObj.hasOwnProperty('method')) {
                    method = jsonObj.method;
                }else if(jsonObj.hasOwnProperty('result')){
                    if(!!AVT_CREATOR_CENTRAL_API.queue[jsonObj.id]) {
                        method = AVT_CREATOR_CENTRAL_API.queue[jsonObj.id];
                        delete AVT_CREATOR_CENTRAL_API.queue[jsonObj.id];
                    }
                }
                if(method !== '') {
                    switch (method) {
                        case 'ax.register.widget.result':
                        case 'ax.register.property.result':
                            events.emit('connected', {
                                connection: websocket,
                                port: inPort,
                                uuid: inUUID
                            });
                            break;

                        default:
                            events.emit(method, jsonObj);
                            break;
                    }
                }
            };

        }

        return {
            connect: connect,
            uuid: inUUID,
            on: (event, callback) => events.on(event, callback),
            emit: (event, callback) => events.emit(event, callback),
            connection: websocket
        };
    }

    return init();
})();
