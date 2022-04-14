package orm

import (
	"errors"
	"fmt"
	"gorm.io/driver/mysql"
	"gorm.io/gorm"
	"web/model"
)

var db *gorm.DB

func CreateCnn() (err error) {
	username := "root" //账号

	password := "123456" //密码
	host := "127.0.0.1"  //数据库地址，可以是Ip或者域名
	port := 3306         //数据库端口
	Dbname := "minioj"   //数据库名

	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?charset=utf8&parseTime=True&loc=Local", username, password, host, port, Dbname)
	//连接MYSQ

	db, err = gorm.Open(mysql.Open(dsn), &gorm.Config{})

	if err != nil {
		panic("连接数据库失败, error=" + err.Error())
	}
	return
}

func InsertProblem(problem *model.Problem) (err error) {
	if err := db.Create(problem).Error; err != nil {
		fmt.Println("插入失败", err)
		return
	}
	return
}

func UpdateProblem(problem *model.Problem) (err error) {
	//更新
	//自动生成Sql: UPDATE `users` SET `password` = '654321'  WHERE (username = 'tizi365')

	db.Model(&model.Problem{}).Where("username = ?", problem.Pid).Updates(&problem) //updates  sss
	return
}

func QueryProblem(Pid int) (err error) {
	u := model.Problem{}
	//自动生成sql： SELECT * FROM `users`  WHERE (username = 'tizi365') LIMIT 1
	result := db.Where("pid = ?", Pid).First(&u)
	if errors.Is(result.Error, gorm.ErrRecordNotFound) {
		fmt.Println("找不到记录")
		return
	}

	return
}
