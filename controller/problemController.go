package controller

import (
	"github.com/gin-gonic/gin"
	"strconv"
	"web/model"
	"web/servers"
)

func AddProblem(c *gin.Context) {
	var pro *model.Problem
	if err := c.ShouldBindJSON(&pro); err != nil {
		c.JSON(200, "类型错误")
		return
	}
	if err := servers.AddProblem(pro); err != nil {
		c.JSON(200, "数据库错误")
		return

	}
	c.JSON(200, "题目上传成功")
}

func ModifyProblem(c *gin.Context) {
	var pro *model.Problem
	if err := c.ShouldBindJSON(&pro); err != nil {
		c.JSON(200, "类型错误")
		return
	}
	if err := servers.ModifyProblem(pro); err != nil {
		c.JSON(200, "数据库错误")
		return

	}
	c.JSON(200, "题目更新成功")
}

func GetProblem(c *gin.Context) {

	pid := c.Param("id")

	proId, _ := strconv.Atoi(pid)
	pro, err := servers.GetProblem(proId)
	if err == nil {
		c.JSON(200, "not find problem")
	}
	c.JSON(200, pro)
}
