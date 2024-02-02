const path = document.querySelector('.path');
const pattern = document.querySelector('.pattern');
const search = document.querySelector('.search');
const messageContainer = document.querySelector('.message-container');
const message = document.querySelector('.message');
const ol = document.querySelector('ol');


search.addEventListener('click', function (ev) {

    const pathValue = path.value;
    const patternValue = pattern.value;

    const pathHasWhiteSpace = /\S/.test(pathValue);
    const patternHasWhiteSpace = /\S/.test(patternValue);

    if (pathValue && patternValue && pathHasWhiteSpace && patternHasWhiteSpace) {

        messageContainer.style.display = 'flex';

        ipcRenderer.send("pipe", pathValue + ' ' + patternValue);
        ipcRenderer.receive('pipe', (data) => {

            if (data.length === 0) {
                messageContainer.style.justifyContent = 'center';
                messageContainer.style.alignItems = 'center';
                message.innerHTML = "There is no such directory";
            } else {

                const splitedData = Array.from(new Set(data.split("\n")));
                console.log(splitedData);

                const matches = splitedData[splitedData.length - 1].split(" ")[3];
                const total = splitedData[splitedData.length - 2].split(" ")[3];


                ol.innerHTML = "";
                if (splitedData.length > 1) {
                    splitedData.forEach(function (item, i) {
                        if (i < splitedData.length - 2) {
                            const html = '<li><p>' + item + '</p></li>';
                            ol.insertAdjacentHTML('beforeend', html);
                        }
                    })


                    if(matches == 0) {
                        message.innerHTML = "No result";
                        messconsole.log(splitedData);ageContainer.style.justifyContent = 'center';
                        messageContainer.style.alignItems = 'center';
                    }else {
                        message.innerHTML = "Total files searched: " + total + "\nTotal matches: " + matches;
                        messageContainer.style.justifyContent = 'flex-start';
                        messageContainer.style.alignItems = 'flex-start';
                    }
                } else {
                    messageContainer.style.justifyContent = 'center';
                    messageContainer.style.alignItems = 'center';
                    mess = "No result"
                }
            }
        })
    }
})




