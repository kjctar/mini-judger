package model

type Problem struct {
	Pid         string `form:"pid" json:"pid" xml:"pid"  gorm:"column:pid" binding:"required"`
	Title       string `form:"title" json:"title" xml:"title" gorm:"column:title" binding:"required"`
	Timelimit   int    `form:"timelimit" json:"timelimit" xml:"timelimit"  gorm:"column:timelimit" binding:"required"`
	Memorylimit int    `form:"memorylimit" json:"memorylimit" xml:"memorylimit" gorm:"column:memorylimit" binding:"required"`
	Createtime  int64  `form:"createtime" json:"createtime" xml:"createtime" gorm:"column:createtime" binding:"required"`
	Author      string `form:"author" json:"author" xml:"author" gorm:"column:author" binding:"required"`
	Content     string `form:"content" json:"content" xml:"content" gorm:"column:content" binding:"required"`
	Intest      string `form:"intest" json:"intest" xml:"intest" gorm:"column:intest" binding:"required"`
	Outest      string `form:"outest" json:"outest" xml:"outest" gorm:"column:outest" binding:"required"`
	Hint        string `form:"hint" json:"hint" xml:"hint" gorm:"column:hint" binding:"required"`
}
