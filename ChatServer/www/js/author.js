// 作者介绍页 —— 顶部“回到”按钮：返回首页
(function () {
    const backBtn = document.getElementById("back-btn");
    if (backBtn) {
        backBtn.addEventListener("click", function () {
            window.location.href = "/pages/home.html";
        });
    }
})();