/* @yehang@sjtu.edu.cn
 * c++版本的redis接口, 自动管理连接释放。
 * 所有接口设计按照redis终端操作来设计，接口方法全部小写
 * 如果方法返回一个数值或者判断，那么将结果作为方法返回值，否则以输出参数的形式进行返回
 * @2019.06.20
 * reids版本要求2.8.12+
 * 注意：此接口可以单独使用，单独使用时请用无参构造函数。
 * 建议配合redis_connect_pool使用
 * */
#pragma once

#include <string>
#include <string.h>//strcmpcase
#include <map>
#include <vector>
#include <sstream>
#include "hiredis.h"

class Redis
{

public:
    Redis();
    Redis(redisContext *context);
    virtual ~Redis();

    void init(const std::string& ip, int port, const std::string& pwd);
    void connect();
    void disconnect();
	
    //---> 基本接口
    //删除key,返回被删除key的个数, 0则表示没有这个key。
    int64_t del(const std::string& key);

    //key是否存在
    bool exists(const std::string& key);

    //设置 key 的过期时间。
    bool expire(const std::string& key, int64_t seconds);
  
    //以 unix timestamp格式设置 key 的过期时间。key 过期后将不再可用。
    bool expireat(const std::string& key, int64_t timestamp);//TODO

    //查找所有符合给定模式 pattern 的 key
    bool keys(const std::string& pattern, std::vector<std::string>& key_list);//yhh

    //返回 key 的剩余过期时间，调用失败返回 -3。
    //当 key 不存在时，返回 -2 。当 key 存在但没有设置剩余生存时间时，返回 -1 。
    int64_t ttl(const std::string& key);
	
    //检测key的类型，输出参数：none(key not exist), string, list, set, zset, hash
    std::string type(const std::string& key);//yhh
    //--->基本接口end
	
	
    //---> string 类型接口

    //覆盖式设置键值对
    bool set(const std::string& key, const std::string& value);

    //key不存在则 value = ""
    std::string get(const std::string& key);

    //批量设置,请保证keys.size()==values.size()
    bool mset(const std::vector<std::string> & keys, const std::vector<std::string> & values);
	
    //批量获取，values可以有初始值，但是会被清空
    bool mget(const std::vector<std::string> & keys, std::vector<std::string> & values);
	
    //key对应的值必须是数字类型，值自增1，非数字类型不会做任何修改，返回0，key不存在则创建，默认初始值为0,再操作。
    int64_t incr(const std::string& key);
	
    //key对应的数字值自增incr，可以是负数，返回新值
    int64_t incrby(const std::string& key, int64_t incr);
	
    //健对应的值自增，不存在则先创建，默认值为0
    float incrbyfloat(const std::string& key, float incr);
	
    //健对应的值自减
    int64_t decr(const std::string& key);
    int64_t decrby(const std::string& key, int64_t decr);
    //--->string 类型接口 end
   
    //--->有序集合 zset 类型接口
	
    //zadd zset1 score item; 返回 成功添加的新成员数量，不包括更新的和已经存在的。调用错误返回-1。
    int64_t zadd(const std::string& key, const std::map<std::string, std::string>& members);
    
    //返回zset中元素个数，调用错误返回-1。
    int64_t zcard(const std::string& key);
    
    //分数在[min, max]之间元素个数。调用错误返回-1。
    int64_t zcount(const std::string& key, double min, double max);
   
    //zset 的member对应的分数加上incr，返回member的新分数，string类型
    std::string  zincrby(const std::string& key, double incr, const std::string& member);

    //values=有序集合zset的values，按照分数从小到大，默认不带scores, 如果带了，values是value,score,value,score这种形式
    bool zrange(const std::string& key, int64_t start, int64_t stop, std::vector<std::string>& values, bool with_scores=false);
    //按照分数从大到小，默认不带scores
	bool zrevrange(const std::string& key, int64_t start, int64_t stop, std::vector<std::string>& values, bool with_scores=false);

    //删除指定下标区间的元素，返回被删除元素个数，调用错误返回-1
    int64_t zremrangebyrank(const std::string& key, int64_t start, int64_t stop);

    //删除zset分数区间的元素，返回被删除元素个数，调用错误返回-1
    int64_t zremrangebyscore(const std::string& key, double min, double max);

    //返回zset的member的分数，如果该zset没有这个member，返回""
    std::string zscore(const std::string& key, const std::string& member);
	
    
    //Redis Zscan 命令用于迭代有序集合中的元素（包括元素成员和元素分值）
    //返回的每个元素都是一个有序集合元素，
    // 一个有序集合元素由一个成员（member）和一个分值（score）组成。
    //count<0 表示返回所有 TODO
    bool zscan(const std::string& key, int64_t cursor, const std::string& pattern, int64_t count,
                std::vector<std::string> & values);
    //--->zset 类型方法 end


    //--->哈希 hash 类型方法
	
    //hash 批量获取所有键值对，初始values为空
    bool hgetall(const std::string& key, std::map<std::string, std::string>& value);
    //--->hash 类型方法 end


    //--->列表 list 类型方法
    //list 获取下标[start, end]区间的元素，end = -1 表示末尾
    bool lrange(const std::string& key, int64_t start, int64_t end, std::vector<std::string>& value);   
    //--->list 类型方法 end
    

    //--->通用接口
    //执行不需要返回值的操作
    bool exec(const std::string& cmd);
    //执行任意redis操作，如果有返回值，放在vector<string>里面
    bool exec(const std::string& cmd, std::vector<std::string>& values);
    //--->通用接口结束

	
    //--->辅助方法
	
    //数字转字符串
    //demo: int a = str2num<int>("-01.23");
    template<class T>
    T str2num(const std::string& str)
    {
        std::stringstream ss;
        T num;
        ss << str;
        ss >> num;
        return num;
    }

    //字符串转数值
    //demo: double num=-23.1; string str = num2str(num);
    //建议指定T的类型，str = num2str<int>(num);
    template<class T>
    std::string num2str(const T& num)
    {
        std::stringstream ss;
        ss << num;
        std::string str;
        ss >> str;
        return str;
    }
    //--->辅助方法结束

private:
    bool execReplyStatus(const std::string& cmd);
    bool execReplyString(const std::string& cmd, std::string& ret);
    bool execReplyInt(const std::string& cmd, int64_t& ret);
    bool execReplyArray(const std::string& cmd, std::vector<std::string>& ret); 
    bool auth(const std::string& pwd);
    void freeReply();
    void freeReply(redisReply * reply);
    bool isError(redisReply * reply);

private:
    redisContext * _context;
    redisReply * _reply;
    std::string _ip;
    int32_t _port;
    std::string _pwd;
    bool _connected;//是否已经连接到redis服务端
};

