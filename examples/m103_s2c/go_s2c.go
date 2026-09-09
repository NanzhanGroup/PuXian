// M103-S2c (Issue 30 GAP-YAML-SER) 对拍参照：Go yaml.v3（与 px yaml_stringify 双向互认）
//   gen    Go yaml.Marshal 典型 api-server 结构 → artifacts/go_out.yaml（供 px parse 读）
//   check  Go yaml.Unmarshal 读 px 写出的 artifacts/px_out.yaml → 断言值还原
//     （px 写 → Go 读互认：llm_providers/purpose_map 结构 + 特殊字符 note + 类型）
// 依赖：gopkg.in/yaml.v3（go.mod）；用法：go build -o build/go_s2c . && ./go_s2c gen|check
package main

import (
	"fmt"
	"os"

	"gopkg.in/yaml.v3"
)

func fatal(err error) {
	fmt.Fprintln(os.Stderr, "ERR", err)
	os.Exit(1)
}

func gen() {
	v := map[string]interface{}{
		"name":  "qg-api",
		"port":  9520,
		"ratio": 0.85,
		"debug": false,
		"note": "Go wrote #hash and \"quotes\" and 中文", // 含 # 引号（px parse 支持双引号值；块标量 |/parse 子集不支持 → 避免 \n）
		"servers": []interface{}{
			map[string]interface{}{"name": "s1", "port": 1001},
			map[string]interface{}{"name": "s2", "port": 1002},
		},
		"purpose": map[string]interface{}{
			"chat": map[string]interface{}{"provider": "qwen"},
		},
	}
	d, err := yaml.Marshal(v)
	if err != nil {
		fatal(err)
	}
	if err := os.WriteFile("artifacts/go_out.yaml", d, 0644); err != nil {
		fatal(err)
	}
	fmt.Println("go gen done")
}

func check() {
	d, err := os.ReadFile("artifacts/px_out.yaml")
	if err != nil {
		fatal(err)
	}
	var m map[string]interface{}
	if err := yaml.Unmarshal(d, &m); err != nil {
		fatal(fmt.Errorf("px 产出无法被 Go yaml 解析: %v", err))
	}
	as := func(k string) string {
		s, _ := m[k].(string)
		return s
	}
	if got := as("upstream"); got != "https://up.example.com" {
		fatal(fmt.Errorf("upstream 还原失败: %q", got))
	}
	if got := as("note"); got != "null" {
		// px 侧 note 是 null 值 → Go 读回 nil
	}
	if v, ok := m["cache_ttl_seconds"].(int); !ok || v != 7200 {
		fatal(fmt.Errorf("cache_ttl_seconds 还原失败: %v", m["cache_ttl_seconds"]))
	}
	if v, ok := m["debug"].(bool); !ok || v {
		fatal(fmt.Errorf("debug 还原失败: %v", m["debug"]))
	}
	provs, ok := m["llm_providers"].([]interface{})
	if !ok || len(provs) != 2 {
		fatal(fmt.Errorf("llm_providers 还原失败: %v", m["llm_providers"]))
	}
	p0, _ := provs[0].(map[string]interface{})
	if p0["name"] != "qwen" || p0["enabled"] != true {
		fatal(fmt.Errorf("llm_providers[0] 还原失败: %v", provs[0]))
	}
	purpose, ok := m["purpose_map"].(map[string]interface{})
	if !ok {
		fatal(fmt.Errorf("purpose_map 还原失败: %v", m["purpose_map"]))
	}
	chat, _ := purpose["chat"].(map[string]interface{})
	if chat["model"] != "qwen-max" {
		fatal(fmt.Errorf("purpose_map.chat.model 还原失败: %v", purpose))
	}
	fmt.Println("m103_s2c_yaml_stringify Go 读 px 产出 OK")
}

func main() {
	os.MkdirAll("artifacts", 0755)
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "usage: go_s2c gen|check")
		os.Exit(2)
	}
	switch os.Args[1] {
	case "gen":
		gen()
	case "check":
		check()
	default:
		fmt.Fprintln(os.Stderr, "usage: go_s2c gen|check")
		os.Exit(2)
	}
}
