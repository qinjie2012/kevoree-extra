package org.kevoree.serializer
import org.kevoree._
trait NodeLinkSerializer 
 extends NetworkPropertySerializer {
def getNodeLinkXmiAddr(selfObject : NodeLink,previousAddr : String): Map[Object,String] = {
var subResult = Map[Object,String]()
var i = 0
i=0
selfObject.getNetworkProperties.foreach{ sub => 
subResult +=  sub -> (previousAddr+"/@networkProperties."+i) 
subResult = subResult ++ getNetworkPropertyXmiAddr(sub,previousAddr+"/@networkProperties."+i)
i=i+1
}
subResult
}
def NodeLinktoXmi(selfObject : NodeLink,refNameInParent : String, addrs : Map[Object,String]) : scala.xml.Node = {
new scala.xml.Node {
  def label = refNameInParent
    def child = {        
       var subresult: List[scala.xml.Node] = List()  
selfObject.getNetworkProperties.foreach { so => 
subresult = subresult ++ List(NetworkPropertytoXmi(so,"networkProperties",addrs))
}
      subresult    
    }              
override def attributes  : scala.xml.MetaData =  { 
var subAtts : scala.xml.MetaData = scala.xml.Null
if(selfObject.getNetworkType.toString != ""){
subAtts= subAtts.append(new scala.xml.UnprefixedAttribute("networkType",selfObject.getNetworkType.toString,scala.xml.Null))
}
if(selfObject.getEstimatedRate.toString != ""){
subAtts= subAtts.append(new scala.xml.UnprefixedAttribute("estimatedRate",selfObject.getEstimatedRate.toString,scala.xml.Null))
}
if(selfObject.getLastCheck.toString != ""){
subAtts= subAtts.append(new scala.xml.UnprefixedAttribute("lastCheck",selfObject.getLastCheck.toString,scala.xml.Null))
}
subAtts}
  }                                                  
}
}
