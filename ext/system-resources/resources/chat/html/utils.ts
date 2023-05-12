export function post(url: string, data: any) {
    var request = new XMLHttpRequest();
    request.open('POST', url, true);
    request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
    request.send(data);
}

function emulate(type: string, detail = {}) {
    const detailRef = {
        type,
        ...detail
    };

    window.dispatchEvent(new CustomEvent('message', {
        detail: detailRef
    }));
}

(window as any)['emulate'] = emulate;

(window as any)['demo'] = () => {
    emulate('ON_MESSAGE', {
        message: {
            args: [ 'me', 'hello!' ]
        }
    })

    emulate('ON_SCREEN_STATE_CHANGE', {
        shouldHide: false
    });
};