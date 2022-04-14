package router

import (
	"github.com/gin-gonic/gin"
	"net/http"
	"web/controller"
)

func SetupRouter(mode string) *gin.Engine {
	r := gin.New()
	r.LoadHTMLFiles("templates/index.html") // 加载html
	r.Static("/static", "./static")         // 加载静态文件
	r.GET("/", func(context *gin.Context) {
		context.HTML(http.StatusOK, "index.html", nil)
	})
	problem := r.Group("/api/problem")
	problem.POST("/add", controller.AddProblem)
	problem.POST("/modify", controller.ModifyProblem) // 注册业务路由
	problem.GET("/content", controller.GetProblem)

	return r
}
