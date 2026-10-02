(function () {
    // 从 URL 读取首页传入的用户发问 prompt
    const params = new URLSearchParams(window.location.search);
    const prompt = params.get("q") || "";

    const usernameEl = document.getElementById("username");
    const passwordEl = document.getElementById("password");
    const errorEl = document.getElementById("error-msg");

    function showError(message) {
        errorEl.textContent = message;
        errorEl.style.display = "block";
    }

    function clearError() {
        errorEl.textContent = "";
        errorEl.style.display = "none";
    }

    function submit() {
        const username = usernameEl.value.trim();
        const password = passwordEl.value;

        if (!username || !password) {
            showError("请填写你的名字和密码");
            return;
        }
        clearError();

        fetch("/api/register", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ username: username, password: password })
        })
            .then(function (res) { return res.json(); })
            .then(function (res) {
                if (res.success) {
                    const query = "username=" + encodeURIComponent(username) +
                        (prompt ? "&q=" + encodeURIComponent(prompt) : "");
                    window.location.href = "/pages/chatroom.html?" + query;
                } else {
                    showError(res.message || "登录/注册失败，请重试");
                }
            })
            .catch(function () {
                showError("网络错误，请稍后重试");
            });
    }

    document.getElementById("submit-btn").addEventListener("click", submit);

    usernameEl.addEventListener("keydown", function (e) {
        if (e.key === "Enter") { submit(); }
    });
    passwordEl.addEventListener("keydown", function (e) {
        if (e.key === "Enter") { submit(); }
    });
})();