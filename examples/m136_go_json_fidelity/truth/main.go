// m136 truth — Go 侧真值（第 16 轮：JSON 数字/字符串/浮点的线上形态）
//
// 为什么要有它：M136 修了 4 处「JSON 往返」类缺陷（int 溢出夹钳、`\u0000` 截断、
//   浮点非最短往返、str() 截断），这些的**唯一权威**是 Go 的 encoding/json。
//   本程序把 Go 的 Unmarshal→Marshal 结果逐行打印，供 verify.sh 与 PuXian 侧逐字节对拍。
//
// 用法： go run ./examples/m136_go_json_fidelity/truth
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
)

var corpus = []string{
	`{"s":"a\u0000b\u001fc","z":"<&>\u00e9\u4e2d"}`,
	`{"b":12345678901234567890}`,
	`{"b":-12345678901234567890}`,
	`{"n":9223372036854775807}`,
	`{"n":-9223372036854775808}`,
	`{"f":0.7}`,
	`{"f":0.1}`,
	`{"f":1e-7}`,
	`{"f":1e-6}`,
	`{"f":1e20}`,
	`{"f":1e21}`,
	`{"f":123456789.123}`,
	`{"f":9007199254740993}`,
	`{"f":9007199254740992}`,
	`{"f":1.7976931348623157e308}`,
	`{"f":5e-324}`,
	`{"f":-0.0}`,
	`{"f":100000}`,
	`{"f":250.0}`,
	`{"f":3.141592653589793}`,
	`{"f":0.30000000000000004}`,
	`{"a":[1,2,{"c":true}],"empty":"","zero":0,"t":true,"nil":null}`,
	`{"big":[1,12345678901234567890,[2,3.5]]}`,
}

func main() {
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	for _, in := range corpus {
		var v interface{}
		if err := json.Unmarshal([]byte(in), &v); err != nil {
			fmt.Fprintf(w, "ERR %v\n", err)
			continue
		}
		b, err := json.Marshal(v)
		if err != nil {
			fmt.Fprintf(w, "MERR %v\n", err)
			continue
		}
		fmt.Fprintf(w, "%s\n", string(b))
	}
}
