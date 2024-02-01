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

            console.log(data);
            if (data.includes('No such directory')) {

                messageContainer.style.justifyContent = 'center';
                messageContainer.style.alignItems = 'center';
                message.innerHTML = "There is no such directory";

            } else {
                const totalFiles = (data.match(new RegExp('@', 'g')) || []).length;
                const totalMathes = (data.match(new RegExp('&', 'g')) || []).length;

                let mess = '';
                mess = "Total Files Searched: " + totalFiles + "\nTotal Matches: " + totalMathes + "\n\n";


                data = data.replace(/@/g, '');
                data = data.replace(/&/g, '');
                data = data.replace(/\n/g, '');

                const splitData = data.split('|');

                ol.innerHTML = "";
                console.log(splitData)
                if (splitData.length > 1) {
                    splitData.forEach(function (item, i) {
                        if (i !== splitData.length - 1) {
                            // mess += i + ":" + item + "\n\n";
                            const html = '<li><p>' + item + '</p></li>';
                            ol.insertAdjacentHTML('beforeend', html);
                        }
                    })
                    messageContainer.style.justifyContent = 'flex-start';
                    messageContainer.style.alignItems = 'flex-start';
                } else {
                    messageContainer.style.justifyContent = 'center';
                    messageContainer.style.alignItems = 'center';
                    mess = "No result"
                }
                message.innerHTML = mess;
            }
        })
    }
})



