#[derive(Clone, Debug)]
pub enum RedisProto {
    SimpleString(String),
    SimpleErr(String),
    Int(i64),
    BulkString(String),
    Array(Vec<RedisProto>),
    Bool(bool),
    Double(f64),
    BigInt(i128),
    BulkErr(String),
    Null,
}
