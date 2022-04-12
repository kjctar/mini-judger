package main

import (
	"github.com/gin-gonic/gin"
	"net/http"
)

func main() {
	// router := gin.Default()

	r := gin.Default()

	r.GET("/", func(c *gin.Context) {
		c.String(http.StatusOK, "Who are you?")
	})
	//r.GET("/user/:name", func(c *gin.Context) {
	//	name := c.Param("name")
	//	c.String(http.StatusOK, "Hello %s", name)
	//	c.Redirect(http.StatusMovedPermanently, "/index")
	//})
	//r.GET("/users", func(c *gin.Context) {
	//	name := c.Query("name")
	//	role := c.DefaultQuery("role", "teacher")
	//	c.String(http.StatusOK, "%s is a %s", name, role)
	//})
	//r.POST("/form", func(c *gin.Context) {
	//	username := c.PostForm("username")
	//	password := c.DefaultPostForm("password", "000000") // 可设置默认值
	//	fmt.Printf("%s %s", username, password)
	//	c.JSON(http.StatusOK, gin.H{
	//		"username": username,
	//		"password": password,
	//	})
	//})
	//r.POST("/upload1", func(c *gin.Context) {
	//	file, _ := c.FormFile("file")
	//	// c.SaveUploadedFile(file, dst)
	//	c.String(http.StatusOK, "%s uploaded!", file.Filename)
	//})
	//r.POST("/upload2", func(c *gin.Context) {
	//	// Multipart form
	//	form, _ := c.MultipartForm()
	//	files := form.File["upload[]"]
	//
	//	for _, file := range files {
	//		log.Println(file.Filename)
	//		// c.SaveUploadedFile(file, dst)
	//	}
	//	c.String(http.StatusOK, "%d files uploaded!", len(files))
	//})
	//type student struct {
	//	Name string
	//	Age  int8
	//}
	//
	//r.LoadHTMLGlob("templates/*")
	//
	//stu1 := &student{Name: "kjctar", Age: 20}
	//stu2 := &student{Name: "Jack", Age: 22}
	//r.GET("/arr", func(c *gin.Context) {
	//	c.HTML(http.StatusOK, "arr.html", gin.H{
	//		"title":  "Gin",
	//		"stuArr": [2]*student{stu1, stu2},
	//	})
	//})

	r.Run(":9999") // listen and serve on 0.0.0.0:8080
}
