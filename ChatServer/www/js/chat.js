(function () {
    "use strict";

    // ===== 读取 URL 参数（home / register 页传入） =====
    const params = new URLSearchParams(window.location.search);
    const username = params.get("username") || "访客";
    const initialPrompt = params.get("q") || "";

    // ===== 全局状态 =====
    let currentSessionId = null;   // 当前会话 id，null 表示尚未创建
    let pendingMessage = "";       // 点击发送时暂存的消息

    // ===== DOM 引用 =====
    const welcomeEl = document.getElementById("welcome");
    const welcomeNameEl = document.getElementById("welcome-name");
    const messagesEl = document.getElementById("messages");
    const inputEl = document.getElementById("input");
    const sendBtn = document.getElementById("send-btn");
    const newChatBtn = document.getElementById("new-chat-btn");
    const sessionListEl = document.getElementById("session-list");
    const userNameEl = document.getElementById("user-name");
    const modalEl = document.getElementById("modal");
    const modelGridEl = document.getElementById("model-grid");
    const modalCloseBtn = document.getElementById("modal-close");

    // ===== 初始化用户信息 =====
    welcomeNameEl.textContent = username;
    userNameEl.textContent = username;
    if (initialPrompt) {
        inputEl.value = initialPrompt;
    }

    // ===== Markdown 渲染配置 =====
    if (typeof marked !== "undefined") {
        marked.setOptions({ gfm: true, breaks: true });
    }

    // ===== 工具函数 =====
    function escapeHtml(str) {
        return String(str)
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#39;");
    }

    function renderMarkdown(text) {
        if (typeof marked !== "undefined") {
            try {
                return marked.parse(text);
            } catch (e) {
                return "<p>" + escapeHtml(text).replace(/\n/g, "<br>") + "</p>";
            }
        }
        return "<p>" + escapeHtml(text).replace(/\n/g, "<br>") + "</p>";
    }

    // 代码高亮 + 注入一键复制按钮
    function decorate(container) {
        container.querySelectorAll("pre code").forEach(function (code) {
            if (window.hljs) {
                try {
                    hljs.highlightElement(code);
                } catch (e) { /* 忽略高亮失败 */ }
            }
            addCopyButton(code);
        });
    }

    function addCopyButton(code) {
        const pre = code.parentElement;
        if (!pre || pre.parentElement.classList.contains("code-block")) {
            return;
        }
        const wrapper = document.createElement("div");
        wrapper.className = "code-block";

        const header = document.createElement("div");
        header.className = "code-header";
        const btn = document.createElement("button");
        btn.type = "button";
        btn.className = "copy-btn";
        btn.textContent = "复制";
        btn.addEventListener("click", function () {
            copyText(code.innerText, btn);
        });
        header.appendChild(btn);

        pre.parentNode.insertBefore(wrapper, pre);
        wrapper.appendChild(header);
        wrapper.appendChild(pre);
    }

    function copyText(text, btn) {
        function done(ok) {
            btn.textContent = ok ? "已复制" : "复制失败";
            setTimeout(function () {
                btn.textContent = "复制";
            }, 1500);
        }

        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(text).then(
                function () { done(true); },
                function () { done(false); }
            );
        } else {
            const ta = document.createElement("textarea");
            ta.value = text;
            ta.style.position = "fixed";
            ta.style.opacity = "0";
            document.body.appendChild(ta);
            ta.select();
            try {
                document.execCommand("copy");
                done(true);
            } catch (e) {
                done(false);
            }
            document.body.removeChild(ta);
        }
    }

    function scrollToBottom() {
        messagesEl.scrollTop = messagesEl.scrollHeight;
    }

    // ===== 消息气泡 =====
    function addMessageBubble(role) {
        const bubble = document.createElement("div");
        bubble.className = "msg " + role;
        const content = document.createElement("div");
        content.className = "md-content";
        bubble.appendChild(content);
        messagesEl.appendChild(bubble);
        scrollToBottom();
        return content;
    }

    function addUserMessage(text) {
        const content = addMessageBubble("user");
        content.textContent = text;
        return content;
    }

    function addAssistantMessage() {
        return addMessageBubble("assistant");
    }

    // ===== 视图切换 =====
    function showWelcome() {
        welcomeEl.hidden = false;
        messagesEl.hidden = true;
    }

    function showConversation() {
        welcomeEl.hidden = true;
        messagesEl.hidden = false;
    }

    // ===== SSE 流式解析 =====
    // 后端用 Json::valueToQuotedString 包裹每个分片，data: 后面是 JSON 字符串字面量，
    // 需 JSON.parse 还原真实文本；结束标识为 data: [DONE]
    function parseSseEvent(rawEvent) {
        const lines = rawEvent.split("\n");
        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            if (line.indexOf("data:") !== 0) {
                continue;
            }
            const payload = line.slice(5).trim();
            if (payload === "[DONE]") {
                return null;
            }
            try {
                return JSON.parse(payload);
            } catch (e) {
                return payload;
            }
        }
        return "";
    }

    async function streamReply(sessionId, message, contentEl) {
        let full = "";
        try {
            const res = await fetch("/api/stream", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ session_id: sessionId, message: message })
            });
            if (!res.ok || !res.body) {
                throw new Error("流式请求失败，状态码 " + res.status);
            }
            const reader = res.body.getReader();
            const decoder = new TextDecoder("utf-8");
            let buffer = "";
            while (true) {
                const chunk = await reader.read();
                if (chunk.done) {
                    break;
                }
                buffer += decoder.decode(chunk.value, { stream: true });
                buffer = buffer.replace(/\r\n/g, "\n");
                let idx;
                while ((idx = buffer.indexOf("\n\n")) !== -1) {
                    const rawEvent = buffer.slice(0, idx);
                    buffer = buffer.slice(idx + 2);
                    const delta = parseSseEvent(rawEvent);
                    if (delta === null) {
                        return;
                    }
                    full += delta;
                    contentEl.innerHTML = renderMarkdown(full);
                    decorate(contentEl);
                    scrollToBottom();
                }
            }
        } catch (e) {
            contentEl.innerHTML = '<div class="err">连接出错：' + escapeHtml(String(e)) + "</div>";
        }
    }

    // ===== 会话相关 =====
    async function createSession(model, message) {
        const res = await fetch("/api/sessions", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({
                model: model,
                session: (message || "新对话").slice(0, 30),
                username: username
            })
        });
        const data = await res.json();
        if (!data.success) {
            throw new Error(data.message || "创建会话失败");
        }
        return data.data.session_id;
    }

    async function refreshSessionList() {
        try {
            const res = await fetch("/api/sessionlists");
            const data = await res.json();
            if (!data.success) {
                return;
            }
            sessionListEl.innerHTML = "";
            (data.data || []).forEach(function (s) {
                renderSessionCard(s);
            });
        } catch (e) { /* 忽略 */ }
    }

    function renderSessionCard(s) {
        const card = document.createElement("div");
        card.className = "session-item" + (s.id === currentSessionId ? " active" : "");

        const title = document.createElement("div");
        title.className = "session-title";
        title.textContent = s.first_user_message || s.model || "新对话";
        title.title = title.textContent;

        const menuBtn = document.createElement("button");
        menuBtn.type = "button";
        menuBtn.className = "session-menu";
        menuBtn.textContent = "⋯";
        menuBtn.title = "更多操作";
        menuBtn.addEventListener("click", function (e) {
            e.stopPropagation();
            toggleSessionMenu(menuBtn, s);
        });

        card.appendChild(title);
        card.appendChild(menuBtn);
        card.addEventListener("click", function () {
            loadSession(s.id);
        });

        sessionListEl.appendChild(card);
    }

    function toggleSessionMenu(btn, s) {
        document.querySelectorAll(".session-dropdown").forEach(function (d) {
            d.remove();
        });

        const menu = document.createElement("div");
        menu.className = "session-dropdown";

        const del = document.createElement("button");
        del.type = "button";
        del.textContent = "删除会话";
        del.addEventListener("click", function () {
            menu.remove();
            deleteSession(s.id);
        });
        menu.appendChild(del);

        const rect = btn.getBoundingClientRect();
        menu.style.position = "fixed";
        menu.style.top = (rect.bottom + 4) + "px";
        menu.style.left = Math.max(8, rect.left - 80) + "px";
        document.body.appendChild(menu);

        setTimeout(function () {
            document.addEventListener("click", function close(e) {
                if (!menu.contains(e.target)) {
                    menu.remove();
                    document.removeEventListener("click", close);
                }
            });
        }, 0);
    }

    async function deleteSession(sessionId) {
        try {
            await fetch("/api/delsession", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ session_id: sessionId })
            });
            if (sessionId === currentSessionId) {
                resetConversation();
            }
            refreshSessionList();
        } catch (e) { /* 忽略 */ }
    }

    async function loadSession(sessionId) {
        try {
            const res = await fetch("/api/session/" + encodeURIComponent(sessionId) + "/history");
            const data = await res.json();
            if (!data.success) {
                return;
            }
            currentSessionId = sessionId;
            messagesEl.innerHTML = "";
            (data.data || []).forEach(function (m) {
                addHistoryMessage(m);
            });
            showConversation();
            scrollToBottom();
            refreshSessionList();
        } catch (e) { /* 忽略 */ }
    }

    function addHistoryMessage(m) {
        const bubble = document.createElement("div");
        bubble.className = "msg " + (m.role === "user" ? "user" : "assistant");
        const content = document.createElement("div");
        content.className = "md-content";
        if (m.role === "user") {
            content.textContent = m.content;
        } else {
            content.innerHTML = renderMarkdown(m.content);
            decorate(content);
        }
        bubble.appendChild(content);
        messagesEl.appendChild(bubble);
    }

    function resetConversation() {
        currentSessionId = null;
        messagesEl.innerHTML = "";
        showWelcome();
        refreshSessionList();
    }

    // ===== 新对话 =====
    newChatBtn.addEventListener("click", function () {
        resetConversation();
        inputEl.focus();
    });

    // ===== 模型选择弹窗 =====
    async function openModal() {
        modalEl.hidden = false;
        modelGridEl.innerHTML = '<div class="modal-loading">加载模型中…</div>';
        try {
            const res = await fetch("/api/models");
            const data = await res.json();
            if (!data.success) {
                throw new Error(data.message || "获取模型失败");
            }
            modelGridEl.innerHTML = "";
            (data.data || []).forEach(function (m) {
                const card = document.createElement("button");
                card.type = "button";
                card.className = "model-card";

                const name = document.createElement("div");
                name.className = "model-name";
                name.textContent = m.name;
                const desc = document.createElement("div");
                desc.className = "model-desc";
                desc.textContent = m.desc || "";
                card.appendChild(name);
                card.appendChild(desc);

                card.addEventListener("click", function () {
                    closeModal();
                    startChat(m.name, pendingMessage);
                });
                modelGridEl.appendChild(card);
            });
        } catch (e) {
            modelGridEl.innerHTML = '<div class="err modal-error">' + escapeHtml(String(e)) + "</div>";
        }
    }

    function closeModal() {
        modalEl.hidden = true;
    }

    modalCloseBtn.addEventListener("click", closeModal);
    modalEl.addEventListener("click", function (e) {
        if (e.target === modalEl) {
            closeModal();
        }
    });

    // ===== 开始对话 =====
    async function startChat(model, message) {
        showConversation();
        addUserMessage(message);
        const aiContent = addAssistantMessage();
        try {
            if (!currentSessionId) {
                currentSessionId = await createSession(model, message);
            }
            await streamReply(currentSessionId, message, aiContent);
            refreshSessionList();
        } catch (e) {
            aiContent.innerHTML = '<div class="err">' + escapeHtml(String(e)) + "</div>";
        }
        inputEl.value = "";
        inputEl.style.height = "auto";
    }

    // ===== 发送入口：点发送 → 先弹模型选择 =====
    function onSend() {
        const text = inputEl.value.trim();
        if (!text) {
            return;
        }
        pendingMessage = text;
        openModal();
    }

    sendBtn.addEventListener("click", onSend);

    inputEl.addEventListener("keydown", function (e) {
        if (e.key === "Enter" && !e.shiftKey) {
            e.preventDefault();
            onSend();
        }
    });

    inputEl.addEventListener("input", function () {
        inputEl.style.height = "auto";
        inputEl.style.height = Math.min(inputEl.scrollHeight, 180) + "px";
    });

    // ===== 初次加载 =====
    refreshSessionList();
})();