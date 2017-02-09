<html>
<body>
    <h1>
    <?el 
        echodb_connect("127.0.0.1", 6666);
        post_len := getenv("CONTENT_LENGTH");
        if (post_len = FALSE) {
            val := echodb_get("welcome");
            if (val = FALSE) {
                val := "Hello World!";
                echodb_set("welcome", val);
            } 
       } else {
            post_len := string_to_int(post_len);
            post := read(post_len);
            val  := parse_query(post, "welcome");
            echodb_update("welcome", val);
        }
        print(val);
    ?>
    </h1>
    <p>如果你能看到这个页面，说明 EchoWeb 已经正常运行</p>
    <p>EchoWeb 由三部分组成:</p>
    <ul>
        <li><strong>EchoS</strong> 一个最简单的 Web 服务器</li>
        <li><strong>EchoL</strong> 一门最简单的 Web 编程语言</li>
        <li><strong>EchoDB</strong> 一个最简单的 KeyValue 型数据库</li>
    </ul>
    <p> 现在，你可以尝试在下方输入新的欢迎语，测试 EchoWeb 对动态内容的处理：</p>
    <form action="index.el" method="post">
        <input type="text" name="welcome" />
        <input type="submit" value="更改" />
    </form>
</body>
