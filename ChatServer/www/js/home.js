(function () {
    // 打字机交替循环文本
    const sentences = [
        "Light spills from this end;",
        "echoes drift from that end."
    ];
    const typeEl = document.getElementById("typewriter-text");
    let sentenceIndex = 0;
    let charIndex = 0;
    let deleting = false;

    function typeLoop() {
        const current = sentences[sentenceIndex];

        if (!deleting) {
            charIndex++;
            typeEl.textContent = current.slice(0, charIndex);
            if (charIndex === current.length) {
                deleting = true;
                setTimeout(typeLoop, 1700);
                return;
            }
            setTimeout(typeLoop, 75);
        } else {
            charIndex--;
            typeEl.textContent = current.slice(0, charIndex);
            if (charIndex === 0) {
                deleting = false;
                sentenceIndex = (sentenceIndex + 1) % sentences.length;
            }
            setTimeout(typeLoop, 42);
        }
    }

    typeLoop();

    // 带着问题参数跳转到注册页
    function jumpToRegister() {
        const question = document.getElementById("question").value.trim();
        const url = "/pages/register.html" +
            (question ? "?q=" + encodeURIComponent(question) : "");
        window.location.href = url;
    }

    document.getElementById("send-btn").addEventListener("click", jumpToRegister);

    document.getElementById("start-btn").addEventListener("click", function () {
        window.location.href = "/pages/register.html";
    });

    document.getElementById("question").addEventListener("keydown", function (e) {
        if (e.key === "Enter" && !e.shiftKey) {
            e.preventDefault();
            jumpToRegister();
        }
    });
})();